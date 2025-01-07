function [coords] = spot_detection_filtered_v4(frame, figures_on, debug_on)
% Questa funzione è progettata per rilevare e identificare la posizione di un punto laser in un'immagine (o "frame") acquisita da una telecamera.

% QUESTA VERSIONE E' QUELLA CHE FUNZIONA IN MODO BUONO SU SFONDI GENERICI MA PRODUCE PATTERN
% AFFETTI DA RIPPLE EVIDENTE!

%% ------------------------------------------------------------ pre-filtraggio ----------------------------------------------------- %%
% Estrae i canali di colore red, green e blue dall'immagine RGB. Il double permette calcoli numerici più precisi
red = double(frame(:, :, 1));
green = double(frame(:, :, 2));
blue = double(frame(:, :, 3));

% Enfatizza le aree in cui il rosso è significativamente più forte rispetto agli altri colori
diff_red = red - (green + blue) / 2;
diff_red(diff_red < 0) = 0; % Imposta a zero i valori negativi per eliminare eventuali disturbi

if debug_on

    figure;
    imshow(mat2gray(red));
    title('Canale Rosso');

    figure;
    imshow(mat2gray(diff_red));
    title('Differenza Canale Rosso - (Verde + Blu)');

end

% 2. Normalizza l'immagine: rende il contrasto uniforme indipendentemente dai valori assoluti dei pixel
normalized_diff = mat2gray(diff_red); % provare ad utilizzare red invece di diff_red. Questa prova la registriamo come processed3

% DEBUG: Visualizza l'immagine normalizzata
if debug_on
    figure;
    imshow(normalized_diff);
    title('Immagine Normalizzata');
end

% 3. Binarizza l'immagine
binary_mask = imbinarize(normalized_diff);

% DEBUG: Visualizza la maschera binaria dopo soglia
if debug_on
    figure;
    imshow(binary_mask);
    title('Maschera Binaria');
end

% 4. Applica operazioni morfologiche per pulire la maschera binaria
binary_mask = imopen(binary_mask, strel('disk', 2)); %  rimuove piccoli oggetti isolati (rumore) usando un elemento strutturante circolare di raggio 2
binary_mask = imclose(binary_mask, strel('disk', 5)); % chiude piccoli buchi all'interno delle regioni connesse, migliorando la qualità delle regioni rilevate

% DEBUG: Visualizza la maschera binaria dopo il filtraggio morfologico
if debug_on
    figure;
    imshow(binary_mask);
    title('Maschera dopo Filtraggio Morfologico');
end
%% ----------------------------------------------------------------------------------------------------------------------------------- %%

% 5. Analisi delle regioni connesse
props = regionprops(binary_mask, 'Area', 'Centroid', 'BoundingBox', 'Eccentricity');

coords = struct('row', NaN, 'col', NaN);

% DEBUG: Mostra bounding box e centroidi su una copia dell'immagine
if debug_on

    debug_image = frame;

    figure;
    imshow(debug_image);
    hold on;

    for i = 1:numel(props)
        rectangle('Position', props(i).BoundingBox, 'EdgeColor', 'y', 'LineWidth', 1);
        plot(props(i).Centroid(1), props(i).Centroid(2), 'bx');
    end

    title('Bounding Box e Centroidi Trovati');
    hold off;

end

% Parametri di filtro
max_eccentricity = 0.9; % per escludere quelle con eccentricità troppo alta (quindi non circolari)
min_area = 10;           % per escludere quelle troppo piccole (Area < 10), che sono probabilmente rumore
min_bbox_size = 10;      % Dimensione minima del bounding box
max_bbox_size = 60;      % Dimensione massima del bounding box
proximity_radius = 140;

valid_region_found = false; % Flag per verificare se una regione valida è stata trovata

% nuove modifiche

if ~isempty(props)
     for i = 1:numel(props)
         area_bbx(i) = props(i).BoundingBox(3) * props(i).BoundingBox(4);
     end
     [~, indx_max_area_bbx] = max(area_bbx);
     centroid_max_area_bbx = [props(indx_max_area_bbx).Centroid(1), props(indx_max_area_bbx).Centroid(2)];
     props(indx_max_area_bbx) = [];
 end

 distances = []; % salvo le distanze fra i centroidi delle regioni connesse residue e il centroide della regione esclusa

 for i = 1:numel(props)
     % Calcola la distanza Euclidea da centroid_max_area_bbx
     candidate_centroid = props(i).Centroid;
     distances = [distances; norm(candidate_centroid - centroid_max_area_bbx)];
 end
 
 false_positive_centroid = find(distances < proximity_radius);

 if(length(false_positive_centroid) == numel(props))
 else
     props(false_positive_centroid) = [];
 end

% fine nuove modifiche

for i = 1:numel(props)

    candidate = props(i);
    if candidate.Eccentricity > max_eccentricity || candidate.Area < min_area || candidate.Area > min_area*1000
        continue; % Escludi regioni non valide
    end

    % Verifica la dimensione del boundig box
    bbox = candidate.BoundingBox;
    width = bbox(3); % Larghezza
    height = bbox(4); % Altezza
    if width < min_bbox_size || width > max_bbox_size || height < min_bbox_size || height > max_bbox_size
        continue;
    end
    
    if(valid_region_found)
        
        radius_old = radius;
        radius_new = sqrt(candidate.Area / pi);

        if(radius_new < radius_old)
            continue;
        end

    else
        % Calcolo del raggio equivalente di un cerchio con la stessa area della regione rilevata
        radius = sqrt(candidate.Area / pi); % formula inversa del calcolo dell'area di un cerchio A = pi * r^2
    end

    
    

    % Controllo della circolarità
    area_to_bbox_ratio = candidate.Area / (pi * (radius ^ 2));
    if abs(area_to_bbox_ratio - 1) > 0.2 % Tolleranza sulla circolarità
        continue;
    end

    % Centroide del cerchio
    Coords.row = candidate.Centroid(2);
    Coords.col = candidate.Centroid(1);

    if ~isnan(Coords.row) && ~isnan(Coords.col)
        % Definisce una ROI (Regione di Interesse): quadrato centrato sul cerchio approssimato
        % xmin = max(floor(Coords.col - radius), 1);
        % xmax = min(ceil(Coords.col + radius), size(frame, 2));
        % ymin = max(floor(Coords.row - radius), 1);
        % ymax = min(ceil(Coords.row + radius), size(frame, 1));
        
        % Definisce una ROI (Regione di Interesse): quadrato inscritto nel cerchio approssimato
        square_side = radius * sqrt(2);

        % Calcolo dei bordi del quadrato
        half_side = square_side / 2;
        xmin = max(floor(Coords.col - half_side), 1);
        xmax = min(ceil(Coords.col + half_side), size(frame, 2));
        ymin = max(floor(Coords.row - half_side), 1);
        ymax = min(ceil(Coords.row + half_side), size(frame, 1));

        roi_red = frame(ymin:ymax, xmin:xmax, 1); % Estrai canale rosso
        
        if debug_on
            figure;
            imshow(roi_red);
            title('roi_red');
        end

        % Applica soglia sul canale rosso
        threshold_value = prctile(roi_red(:), 99);
        roi_mask = roi_red >= threshold_value; % imposta a true tutti i pixel della ROI il cui valore è maggiore o uguale alla soglia calcolata
        % threshold_value = threshold;
        % roi_mask = roi_red >= threshold_value;

        % Calcola il centroide della maschera
        props2 = regionprops(roi_mask, 'Centroid', 'Area', 'Eccentricity');

        if ~isempty(props2)
            % Filtra regioni con eccentricità troppo alta
            valid_regions = [props2.Eccentricity] < 0.8;
            props3 = props2(valid_regions);

            if ~isempty(props3)

                % Trova la regione con l'area massima
                [~, max_idx] = max([props3.Area]);
                refined_row = props3(max_idx).Centroid(2) + ymin - 1;
                refined_col = props3(max_idx).Centroid(1) + xmin - 1;

                coords.row = refined_row;
                coords.col = refined_col;

                valid_region_found = true; % Abbiamo trovato una regione valida

            else
                coords.row = Coords.row;
                coords.col = Coords.col;

                valid_region_found = true; % Abbiamo trovato una regione valida

            end

        end

    end

end

if (valid_region_found == false)
    % Trova la regione con la bounding box più piccola
    areas = arrayfun(@(x) x.BoundingBox(3) * x.BoundingBox(4), props); % applica una funzione anonima che calcola l'area di ciascuna BoundingBox di props
    [~, min_bbox_idx] = min(areas);
    smallest_bbox = props(min_bbox_idx);
    coords.row = smallest_bbox.Centroid(2);
    coords.col = smallest_bbox.Centroid(1);
end

% Visualizzazione del cerchio
if figures_on

    figure;
    imshow(frame);
    hold on;

    % % Sovrappone la ROI come maschera trasparente
    % masked_image = frame;  % copia dell'immagine originale
    % masked_image(repmat(~roi_mask, [1, 1, 3])) = 0;  % imposta a 0 i pixel fuori dalla maschera (nero)
    %
    % % Visualizza la maschera sovrapposta all'immagine
    % imshow(masked_image);  % Immagine con la maschera
    % hold on;

    theta = linspace(0, 2*pi, 100);
    x_circle = Coords.col + radius * cos(theta);
    y_circle = Coords.row + radius * sin(theta);
    plot(x_circle, y_circle, 'r-', 'LineWidth', 2);
    plot(Coords.col, Coords.row, 'gx', 'MarkerSize', 5, 'LineWidth', 2);
    plot(coords.col, coords.row, 'bx', 'MarkerSize', 5, 'LineWidth', 2);
    title('Spot Laser Approssimato con un Cerchio');

    hold off;

end

end

