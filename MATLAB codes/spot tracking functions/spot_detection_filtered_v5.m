function [coords] = spot_detection_filtered_v5(frame, figures_on, debug_on)
% Questa funzione è progettata per rilevare e identificare la posizione di un punto laser in un'immagine (o "frame") acquisita da una telecamera.

% QUESTA VERSIONE E' QUELLA CHE FUNZIONA IN MODO OTTIMO SU SFONDI NERI MA
% NON BENISSIMO SU ALTRI SFONDI!!!!
%% --------------------------------------------------------- pre-filtraggio -----------------------------------------------%%

% Estrai il canale rosso
red_channel = frame(:, :, 1); % Assumi che l'immagine sia RGB

if debug_on
    figure
    % Mostra il canale rosso
    imshow(red_channel);
    title('Canale Rosso');
end

% Applica il filtro Gaussiano
sigma = 2; % Valore di smoothing: più è alto maggiore sarà la levigazione dell'immagine
smoothed_frame = imgaussfilt(red_channel, sigma);

if debug_on
    figure
    % Mostra il risultato
    imshow(smoothed_frame);
    title('Filtro Gaussiano Applicato');
end

% Stima del background usando un'operazione morfologica (imopen)
se = strel('disk', 10); % Elemento strutturante: disco circolare di raggio 10 pixel
background = imopen(smoothed_frame, se);

% Sottrai il background
background_subtracted = imsubtract(smoothed_frame, background);

if debug_on
    figure
    % Mostra il risultato
    imshow(background_subtracted);
    title('Sottrazione del Background');
end

% % Normalizza l'immagine (se necessario)
normalized_frame = mat2gray(background_subtracted);

% % Applica l'equalizzazione dell'istogramma
% contrast_enhanced = adapthisteq(normalized_frame, 'ClipLimit', 0.03);
% 
% if debug_on
%     figure
%     % Mostra il risultato
%     imshow(contrast_enhanced);
%     title('Equalizzazione del Contrasto');
% end

%% ------------------------------------------------------------------------------------------------------------------------------------%%
 %% Identificazione del Pixel Massimo
    [max_value, linear_idx] = max(background_subtracted(:)); % identifico il massimo valore dell'immagine processata
    [row, col] = ind2sub(size(background_subtracted), linear_idx); % Le coordinate del pixel massimo sono convertite in riga e colonna
    
    % Le coordinate vengono salvate nella struttura coords
    coords.row = row;
    coords.col = col;

    %% Visualizzazione
    if figures_on
        figure;
        imshow(background_subtracted, []);
        title('Centro dello Spot Laser');
        hold on;
        plot(col, row, 'r+', 'MarkerSize', 5, 'LineWidth', 2); % Marker rosso sul centro
        hold off;
    end

    %% Debug
    if debug_on
        disp(['Centro dello spot laser: Riga = ', num2str(row), ', Colonna = ', num2str(col)]);
    end
end


