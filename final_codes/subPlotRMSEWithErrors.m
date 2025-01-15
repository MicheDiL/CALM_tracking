function subPlotRMSEWithErrors(data, titolo, xlabel_text)

    % funzione analoga a plotRMSEWithErrors ma che organizza le figure come
    % subplot. data deve però essere una struttura (vedere lo script
    % "display_accuracy2").

    rmse = {data.tot.rmse; data.x.rmse; data.y.rmse};
    errori = {data.tot.errori; data.x.errori; data.y.errori};

    % Indici equispaziati per le barre
    indici = 1:length(data.freq); % Indici da 1 a N (equispaziati)
    
    % Etichette pedici
    pedici_asse_y = {'_{tot}', '_{x}', '_{y}'};

    % Creazione del grafico a barre
    figure;

    for k = 1:3
    
        subplot(3,1,k)

        hBar = bar(indici, rmse{k}, 'FaceColor', 'flat'); % Usa indici per equispaziamento
        set(hBar, 'EdgeColor', 'none'); % Rimuove i contorni delle barre

        hold on;

        % Generazione di colori casuali
        numBarre = length(data.freq); % Numero di barre
        % coloriCasuali = rand(numBarre, 3); % Matrice Nx3 di colori casuali RGB
        % hBar.CData = coloriCasuali; % Assegna colori casuali alle barre
        coloriChiari = pastelColors(numBarre); % Ottieni colori pastello
        hBar.CData = coloriChiari;

        % Aggiunta delle barre di errore
        errorbar(indici, rmse{k}, errori{k}, 'k.', 'LineWidth', 0.5);

        % Aggiunta dei valori delle barre di errore accanto alle barre
        for i = 1:length(data.freq)
            % Posiziona il testo sopra ogni barra
            posizione_y = rmse{k}(i) + errori{k}(i) / 5 + 0.01 * max(rmse{k} + errori{k});
            text(indici(i) + 0.1, posizione_y, ...
                sprintf('%.2f', errori{k}(i)), ... % Valore dell'errore
                'HorizontalAlignment', 'center', ...
                'FontSize', 10, 'FontWeight', 'bold', 'Color', 'black');
        end

        % Adattamento dei limiti dell'asse Y
        yMax = max(rmse{k} + errori{k}); % Valore massimo (RMSE + errore)
        ylim([0, yMax + 0.1 * yMax]); % Margine superiore del 10%
       
        % Calcolo del passo per la griglia
        gridStep = round(yMax / 5, 1); % Suddivisione in 5 intervalli e arrotondo i tick sull'asse y alla prima cifra decimale
        % gridStep = max(round(yMax / 5, 1), 0.01); % Imposta un passo minimo di 0.01
        if gridStep == 0
            gridStep = (yMax / 5);
        else
            yticks(0:gridStep:round(yMax + 0.1 * yMax, 1)); % imposta i tick sull'asse Y da 0 al limite superiore arrotondato con un passo pari a gridStep
            yticklabels(arrayfun(@(x) sprintf('%.1f', x), 0:gridStep:round(yMax + 0.1 * yMax, 1), 'UniformOutput', false)); % Etichette arrotondate
        end

        % Configurazione dell'asse X
        xticks(indici); % Posizioni dei tick
        xticklabels(string(data.freq)); % Etichette dei tick basate sulle frequenze
        
        if k == 3
            % xlabel(xlabel_text, 'FontSize', 10); % Etichetta per l'ultimo subplot
            xlabel(xlabel_text, 'FontSize', 10, 'Interpreter', 'tex'); % Label asse X
        end

        % Configurazione dell'asse Y con pedice e unità di misura
        ylabel(sprintf('%s%s [%s]', 'RMSE', pedici_asse_y{k}, '\mum'), ...
               'FontSize', 10, 'Interpreter', 'tex'); % Usa pedice LaTeX e unità di misura

        % Personalizzazione della griglia
        grid on;
        ax = gca; % Ottieni l'asse corrente
        ax.XGrid = 'off'; % Disattiva griglia verticale
        ax.YGrid = 'on'; % Mantieni griglia orizzontale
        ax.TickLength = [0,0]; % imposta a zero la lunghezza dei trattini sull'asse x

        % Estetica generale
        set(gca, 'FontSize', 10);
        box off; % Rimuove la cornice del grafico

    end

    % Titolo generale della figura
    sgtitle(titolo, 'FontSize', 10, 'FontWeight', 'bold');

end

function colors = pastelColors(numColors)
    % pastelColors: Genera una matrice di colori pastello per le barre
    % numColors - Numero di colori da generare (intero positivo)
    %
    % OUTPUT:
    % colors - Matrice Nx3 di colori pastello (RGB)

    baseColors = [ ...
        240, 163, 255; % Viola
        255, 204, 153; % Arancione
        153, 204, 255; % Azzurro
        153, 255, 204; % Verde chiaro
        255, 153, 204; % Rosa chiaro
        255, 255, 153; % Giallo chiaro
        204, 255, 153; % Verde lime
        204, 204, 255  % Blu chiaro
    ] / 255; % Normalizza su scala [0,1]

    % Cicla i colori per adattarsi al numero richiesto
    colors = repmat(baseColors, ceil(numColors / size(baseColors, 1)), 1);
    colors = colors(1:numColors, :);
end