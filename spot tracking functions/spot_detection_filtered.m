function [coords] = spot_detection_filtered(frame, threshold, crop_on, rect, figures_on)
% Questa funzione è progettata per rilevare e identificare la posizione di un punto laser in un'immagine (o "frame") acquisita da una telecamera.

% La funzione:
%   Rileva il centroide del punto laser nella matrice dell'immagine.
%   Filtra il punto laser basandosi su una soglia di intensità.
%   Ritorna le coordinate (riga e colonna) del punto laser rilevato.
%   Può opzionalmente ritagliare l'immagine e visualizzare il risultato

% La funzione accetta cinque argomenti:
%   frame: L'immagine acquisita in cui cercare il punto laser.
%   threshold: Valore tra 0 e 1 che definisce la soglia di intensità per distinguere il laser dal resto dell'immagine.
%   crop_on: Booleano che specifica se ritagliare l'immagine prima dell'elaborazione.
%   rect: Coordinate e dimensioni della finestra di ritaglio, usate solo se crop_on è attivo.
%   figures_on: Booleano per abilitare o disabilitare la visualizzazione delle immagini durante l'elaborazione.

% La funzione restituisce:
%   coords: Una struttura con i campi:
%       row: Coordinata della riga del centroide del punto laser.
%       col: Coordinata della colonna del centroide del punto laser.
%   Se il laser non è rilevato (ad esempio, l'area del laser è troppo piccola), le coordinate ritornano come NaN.

%%frame = imread(im_path);

if crop_on
    frame = imcrop(frame,rect); % ritaglia l'immagine usando le coordinate di rect
end

% ESTRAZIONE DEL CANALE ROSSO
% L'immagine viene analizzata solo sul canale rosso (spesso il laser emette luce rossa). Se l'immagine è in scala di grigi, non viene effettuata alcuna modifica.
red = frame(:, :, 1); 


% SOGLIA PER IL RILEVAMENTO
% Crea una mappa binaria che indica i pixel con intensità maggiore o uguale a una frazione (threshold) dell'intensità massima nel canale rosso. I
% pixel sopra soglia saranno quelli in cui è possibile sia presente il laser
laser = (red >= (threshold * double(max(red(:))))); 
% laser2 = (red >= (0.25 * double(max(red(:)))));

% % mascehera logica che identidifica le regioni connesse ovvero le aree luminose (quelle in cui è possibile sia presente il laser) nel canale rosso
log_frame = logical(laser);
% log_frame2 = logical(laser2);

% Utilizziamo la funzione regionprops per analizzare le proprietà
% delle regioni connesse (quelle in cui è possibile sia presente il laser) della maschera binaria.Le proprietà estratte includono:
%   Area: numero di pixel della regione
%   Centroide: coordinata centrale della regione
%   BoundigBox: rettangolo che racchiude la regione

% % [x, y] = centroid_logical(log_frame);
% % props = regionprops(bwlabel(laser), 'Area', 'Centroid');
props = regionprops(log_frame,  'Area', 'Centroid','BoundingBox', 'Eccentricity'); 
% props2 = regionprops(log_frame2, 'Area', 'Centroid','BoundingBox');

area = [props.Area];

% Parametri di configurazione
threshold2 = 5;          % Soglia minima per l'area
proximity_radius = 5;    % Raggio per controllare la presenza di centroidi vicini
max_eccentricity = 0.8;  % Soglia per la circolarità: vicino a 0 per forme più circolari
min_bbox_size = 5;       % Dimensione minima del bounding box
max_bbox_size = 50;      % Dimensione massima del bounding box

% Ricerca regione valida
row = NaN;
col = NaN;
box = NaN(1,4); % Inizializza il bounding box come vuoto

% Iterazione sulle regioni connesse
while ~isempty(props)
    % 4.1 Trova la regione con l'area massima
    [num_pixels, index] = max(area);
    candidate = props(index);

    % Verifica se l'area è sufficiente
    if num_pixels <= threshold2
        break;
    end

    % Controlla l'eccentricità
    if candidate.Eccentricity > max_eccentricity
        % Escludi la regione corrente e continua con le altre
        props(index) = []; 
        area(index) = [];
        continue;
    end

    % Verifica la dimensione del boundig box
    bbox = candidate.BoundingBox;
    width = bbox(3); % Larghezza
    height = bbox(4); % Altezza
    if width < min_bbox_size || width > max_bbox_size || height < min_bbox_size || height > max_bbox_size
        % Escludi la regione corrente e continua con le altre
        props(index) = []; 
        area(index) = [];
        continue;
    end
    
    % Calcola la distanza Euclidea dai centroidi vicini in props
    candidate_centroid = candidate.Centroid;
    distances = arrayfun(@(x) norm(candidate_centroid - x.Centroid), props);

    % Escludi la regione se ha centroidi troppo vicini
    close_centroids = distances < proximity_radius;
    close_centroids(index) = false; % Ignora la distanza del candidato con se stesso
    if any(close_centroids)
        % Escludi la regione corrente e continua con le altre
        props(index) = []; 
        area(index) = [];
        continue;
    end

    % Regione valida trovata
    row = candidate_centroid(2);
    col = candidate_centroid(1);
    box = bbox;
    break;
end

%% Create figure and size

if (figures_on)

    % configura la dimensione delle finestre grafiche e la posizione dei grafici sullo schermo
    scrsz = get(groot,'ScreenSize'); % ottieni la dimensione dello schermo
    %                 [left bottom width height]
    W = 750; % Larghezza della finestra
    H = 250; % Altezza della finestra
    
    % mostra un primo blocco di immagini: mostra i canali R, G, B
    figure('Position',[scrsz(3)/2-W/2 scrsz(4)/2 W H]) % crea una finestra centrata
    
    subplot(131); imshow(frame(:,:,1)); title('R'); % mostra il primo canale dell'immagine RGB cioè il canale rosso
    subplot(132); imshow(frame(:,:,2)); title('G'); % mostra il secondo canale dell'immagine RGB cioè il canale verde
    subplot(133); imshow(frame(:,:,3)); title('B'); % mostra il terzo canale dell'immagine RGB cioè il canale blu
    
    % mostra un secondo blocco di immagini: 
    %   immagine originale con centroide rilevato e rettangolo che racchiude il laser, 
    %   il canale R dell'immagine stessa, 
    %   la maschera binaria usata per identificare il laser
    figure('Position',[scrsz(3)/2-W/2 scrsz(4)/2-1.2*H W H]) % crea un'altra finestra
    
    subplot(131);
    imshow(frame);
    title('a) Original Frame', 'Interpreter', 'latex', 'FontName', 'Times New Roman');
    hold on;
    if ~isnan(row) && ~isnan(col)
        plot(col, row, 'gx', 'LineWidth', 2);
        rectangle('Position', [box(1), box(2), box(3), box(4)], 'EdgeColor', 'g', 'LineWidth', 2);
    end

    subplot(132);
    imshow(red);
    title('b) Red Channel', 'Interpreter', 'latex', 'FontName', 'Times New Roman');
    subplot(133);
    imshow(log_frame);
    title('c) Detected Laser Spot', 'Interpreter', 'latex', 'FontName', 'Times New Roman');
    hold on;
    if ~isnan(row) && ~isnan(col)
        plot(col, row, 'gx', 'LineWidth', 2);
        rectangle('Position', [box(1), box(2), box(3), box(4)], 'EdgeColor', 'g', 'LineWidth', 2);
    end

end

coords.row = row;
coords.col = col;

end
