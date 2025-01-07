function [coords] = spot_detection_filtered_v6(frame, figures_on, debug_on)

% Questa funzione è progettata per rilevare e identificare la posizione di un punto laser in un'immagine (o "frame") acquisita da una telecamera.

% VERSIONE OTTIMIZZATA: OTTIMO FUNZIONAMENTO SU SFONDI NERI. FUNZIONA BENE
% A PATTO DI AVER OASCURATO IL RIFLESSO DELLO SPECCHIO NEL CAMPO VISIVO
% DELLA CAMERA

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
normalized_diff = mat2gray(diff_red); 

% DEBUG: Visualizza l'immagine normalizzata
if debug_on
    figure;
    imshow(normalized_diff);
    title('Immagine Normalizzata');
end

% 3. Binarizza l'immagine
threshold = 0.2; % Seleziona una soglia (da 0 a 1)
binary_mask = normalized_diff > threshold; % Mantieni solo i pixel più luminosi
% binary_mask = imbinarize(normalized_diff);

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

% inizio pezzo nuovo: controllo su props
if isempty(props)
    warning('Nessuna regione valida trovata nella maschera binaria.');
    coords.row = NaN;
    coords.col = NaN;
    return; % Esci dalla funzione
end
% fine pezzo nuovo

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
max_eccentricity = 0.9;  % per escludere quelle con eccentricità troppo alta (quindi non circolari)
min_area = 10;           % per escludere quelle troppo piccole (Area < 10), che sono probabilmente rumore
min_bbox_size = 10;      % Dimensione minima del bounding box
proximity_radius = 140;

valid_region_found = false; % Flag per verificare se una regione valida è stata trovata

for i = 1:numel(props)

    candidate = props(i);
    if candidate.Eccentricity > max_eccentricity || candidate.Area < min_area 
        continue; % Escludi regioni non valide
    end

    % Verifica la dimensione del boundig box
    bbox = candidate.BoundingBox;
    width = bbox(3); % Larghezza
    height = bbox(4); % Altezza
    if width < min_bbox_size || height < min_bbox_size 
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
    
    theta = linspace(0, 2*pi, 100);
    x_circle = Coords.col + radius * cos(theta);
    y_circle = Coords.row + radius * sin(theta);

    if debug_on

        debug_image = frame;

        figure;
        imshow(debug_image);
        hold on;

        for i = 1:numel(props)
            rectangle('Position', props(i).BoundingBox, 'EdgeColor', 'y', 'LineWidth', 1);
            plot(props(i).Centroid(1), props(i).Centroid(2), 'bx');
        end

        plot(x_circle, y_circle, 'r-', 'LineWidth', 2);
        plot(Coords.col, Coords.row, 'gx', 'MarkerSize', 5, 'LineWidth', 2);

        title('Centroide cerchio');
        hold off;

    end

    coords.row = Coords.row;
    coords.col = Coords.col;
    valid_region_found = true; % Abbiamo trovato una regione valida

end



if (valid_region_found == false)
   
    % Trova la regione con la bounding box più piccola
    areas = arrayfun(@(x) x.BoundingBox(3) * x.BoundingBox(4), props); % applica una funzione anonima che calcola l'area di ciascuna BoundingBox di props
    [~, min_bbox_idx] = min(areas);
    smallest_bbox = props(min_bbox_idx);
    coords.row = smallest_bbox.Centroid(2);
    coords.col = smallest_bbox.Centroid(1);
    
    % inizio pezzo nuovo
    Coords.col = coords.col;
    Coords.row = coords.row;
    % fine pezzo nuovo
end

% Visualizzazione del cerchio
if figures_on

    figure;
    imshow(frame);
    hold on;

    % inizio pezzo nuovo
    if(valid_region_found == true)
        theta = linspace(0, 2*pi, 100);
        x_circle = Coords.col + radius * cos(theta);
        y_circle = Coords.row + radius * sin(theta);
        plot(x_circle, y_circle, 'r-', 'LineWidth', 2);
    end
    % fine pezzo nuovo

    plot(Coords.col, Coords.row, 'gx', 'MarkerSize', 5, 'LineWidth', 2);
    plot(coords.col, coords.row, 'bx', 'MarkerSize', 5, 'LineWidth', 2);
    title('Spot Laser Approssimato con un Cerchio');

    hold off;

end

end



