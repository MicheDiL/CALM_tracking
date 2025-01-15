function [coords] = spot_detection_filtered_v2(frame, threshold, figures_on, debug_on)
% Questa funzione è progettata per rilevare e identificare la posizione di un punto laser in un'immagine (o "frame") acquisita da una telecamera.

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
        plot(props(i).Centroid(1), props(i).Centroid(2), 'rx');
    end

    title('Bounding Box e Centroidi Trovati');
    hold off;

end

% Parametri di filtro
max_eccentricity = 0.8; % per escludere quelle con eccentricità troppo alta (quindi non circolari)
min_area = 10; % per escludere quelle troppo piccole (Area < 10), che sono probabilmente rumore

% SOLUZIONE 1
% for i = 1:numel(props)
%     candidate = props(i);
%     if candidate.Eccentricity > max_eccentricity || candidate.Area < min_area
%         continue; % Escludi regioni non valide
%     end
%     coords.row = candidate.Centroid(2);
%     coords.col = candidate.Centroid(1);
%     break;
% end
% 
% % Visualizzazione
% if figures_on
%     figure;
%     imshow(frame);
%     hold on;
%     if ~isnan(coords.row) && ~isnan(coords.col)
%         plot(coords.col, coords.row, 'gx', 'MarkerSize', 10, 'LineWidth', 2);
%     end
%     title('Spot Laser Rilevato');
%     hold off;
% end

% SOLUZIONE 2: approssimare le regioni connesse come cerchi
for i = 1:numel(props)
    candidate = props(i);
    if candidate.Eccentricity > max_eccentricity || candidate.Area < min_area
        continue; % Escludi regioni non valide
    end

    % Calcolo del raggio equivalente di un cerchio con la stessa area della regione rilevata
    radius = sqrt(candidate.Area / pi);

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
        xmin = max(floor(Coords.col - radius), 1);
        xmax = min(ceil(Coords.col + radius), size(frame, 2));
        ymin = max(floor(Coords.row - radius), 1);
        ymax = min(ceil(Coords.row + radius), size(frame, 1));

        roi_red = frame(ymin:ymax, xmin:xmax, 1); % Estrai canale rosso

        % Applica soglia sul canale rosso
        threshold_value = prctile(roi_red(:), 99);
        roi_mask = roi_red >= threshold_value; % imposta a true tutti i pixel della ROI il cui valore è maggiore o uguale alla soglia calcolata
        % threshold_value = threshold;
        % roi_mask = roi_red >= threshold_value;

        % Calcola il centroide della maschera
        props2 = regionprops(roi_mask, 'Centroid', 'Area', 'Eccentricity');
        % [~, max_idx] = max([props2.Area]); % Trova l'indice della regione con area massima
        % refined_row = props2(max_idx).Centroid(2) + ymin - 1;
        % refined_col = props2(max_idx).Centroid(1) + xmin - 1;
        % coords.row = refined_row;
        % coords.col = refined_col;

        if ~isempty(props2)
            % Filtra regioni con eccentricità troppo alta
            valid_regions = [props2.Eccentricity] < 0.8;
            props2 = props2(valid_regions);

            if ~isempty(props)
                % Trova la regione con l'area massima
                [~, max_idx] = max([props2.Area]);
                refined_row = props2(max_idx).Centroid(2) + ymin - 1;
                refined_col = props2(max_idx).Centroid(1) + xmin - 1;
                coords.row = refined_row;
                coords.col = refined_col;

            end

        end

    % Visualizzazione del cerchio
    if figures_on

        figure;
        imshow(frame);
        hold on;

        theta = linspace(0, 2*pi, 100);
        x_circle = Coords.col + radius * cos(theta);
        y_circle = Coords.row + radius * sin(theta);
        plot(x_circle, y_circle, 'r-', 'LineWidth', 2);
        plot(Coords.col, Coords.row, 'gx', 'MarkerSize', 5, 'LineWidth', 2);
        plot(coords.col, coords.row, 'bx', 'MarkerSize', 5, 'LineWidth', 2);
        title('Spot Laser Approssimato con un Cerchio');

        hold off;

    end

    break;
end

% SOLUZIONE 3: uso la soluzione 2 come maschera sul canale rosso e applico
% una soglia di intensità

% for i = 1:numel(props)
%     candidate = props(i);
%     if candidate.Eccentricity > max_eccentricity || candidate.Area < min_area
%         continue; % Escludi regioni non valide
%     end
% 
%     % Calcolo del raggio equivalente di un cerchio con la stessa area della regione rilevata
%     radius = sqrt(candidate.Area / pi);
% 
% 
%     % Controllo della circolarità
%     area_to_bbox_ratio = candidate.Area / (pi * (radius ^ 2));
%     if abs(area_to_bbox_ratio - 1) > 0.2 % Tolleranza sulla circolarità
%         continue;
%     end
% 
%     % Salva il centro approssimato come le coordinate
%     coords.row = candidate.Centroid(2);
%     coords.col = candidate.Centroid(1);
% 
%     if ~isnan(coords.row) && ~isnan(coords.col)
%         % Definisce una ROI (Regione di Interesse): quadrato centrato sul cerchio approssimato
%         xmin = max(floor(coords.col - radius), 1);
%         xmax = min(ceil(coords.col + radius), size(frame, 2));
%         ymin = max(floor(coords.row - radius), 1);
%         ymax = min(ceil(coords.row + radius), size(frame, 1));
% 
%         roi_red = frame(ymin:ymax, xmin:xmax, 1); % Estrai canale rosso
% 
%         % Applica soglia sul canale rosso
%         threshold_value = prctile(roi_red(:), 99);
%         roi_mask = roi_red >= threshold_value; % imposta a true tutti i pixel della ROI il cui valore è maggiore o uguale alla soglia calcolata
%         % threshold_value = threshold;
%         % roi_mask = roi_red >= threshold_value;
% 
%         % Calcola il centroide della maschera
%         props2 = regionprops(roi_mask, 'Centroid', 'Area');
%         [~, max_idx] = max([props2.Area]); % Trova l'indice della regione con area massima
%         refined_row = props2(max_idx).Centroid(2) + ymin - 1;
%         refined_col = props2(max_idx).Centroid(1) + xmin - 1;
%         coords.row = refined_row;
%         coords.col = refined_col;
% 
%         % if ~isempty(props)
%         %     refined_row = props.Centroid(2) + ymin - 1;
%         %     refined_col = props.Centroid(1) + xmin - 1;
%         % 
%         %     % Aggiorna le coordinate
%         %     coords.row = refined_row;
%         %     coords.col = refined_col;
%         % end
%     end
% 
%     % Visualizzazione
%     if figures_on
% 
%         figure;
%         imshow(frame);
%         hold on;
% 
%         if ~isnan(coords.row) && ~isnan(coords.col)
%             % Visualizza il cerchio
%             theta = linspace(0, 2*pi, 100);
%             x_circle = coords.col + radius * cos(theta);
%             y_circle = coords.row + radius * sin(theta);
%             plot(x_circle, y_circle, 'r-', 'LineWidth', 2);
% 
%             % Visualizza il centro raffinato
%             plot(coords.col, coords.row, 'bx', 'MarkerSize', 5, 'LineWidth', 1);
%         end
%         title('Centro Spot Laser Refinato');
%         hold off;
%     end

end