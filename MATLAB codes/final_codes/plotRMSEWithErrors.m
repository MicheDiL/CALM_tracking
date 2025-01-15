function plotRMSEWithErrors(frequenze, rmse, errori, titolo, xlabel_text, ylabel_text)
    % plotRMSEWithErrors: Genera un grafico a barre con RMSE e barre di errore
    % 
    % INPUTS:
    % frequenze    - Array di frequenze [Hz]
    % rmse         - Array dei valori di RMSE
    % errori       - Array delle barre di errore corrispondenti
    % titolo       - Titolo del grafico (stringa)
    % xlabel_text  - Etichetta per l'asse X (stringa)
    % ylabel_text  - Etichetta per l'asse Y (stringa)

    % Indici equispaziati per le barre
    indici = 1:length(frequenze); % Indici da 1 a N (equispaziati)

    % Creazione del grafico a barre
    figure;
    hBar = bar(indici, rmse, 'FaceColor', 'flat'); % Usa indici per equispaziamento
    set(hBar, 'EdgeColor', 'none'); % Rimuove i contorni delle barre

    hold on;

    % Generazione di colori casuali
    numBarre = length(rmse); % Numero di barre
    coloriCasuali = rand(numBarre, 3); % Matrice Nx3 di colori casuali RGB
    hBar.CData = coloriCasuali; % Assegna colori casuali alle barre

    % Aggiunta delle barre di errore
    errorbar(indici, rmse, errori, 'k.', 'LineWidth', 0.5);

    % Aggiunta dei valori delle barre di errore accanto alle barre
    for i = 1:length(frequenze)
        % Posiziona il testo sopra ogni barra
        posizione_y = rmse(i) + errori(i) / 5 + 0.01 * max(rmse + errori); 
        text(indici(i) + 0.1, posizione_y, ...
            sprintf('%.2f', errori(i)), ... % Valore dell'errore
            'HorizontalAlignment', 'center', ...
            'FontSize', 10, 'FontWeight', 'bold', 'Color', 'black');
    end

    % Configurazione dell'asse X
    xticks(indici); % Posizioni dei tick
    xticklabels(string(frequenze)); % Etichette dei tick basate sulle frequenze
    xlabel(xlabel_text, 'FontSize', 12);

    % Configurazione dell'asse Y
    ylabel(ylabel_text, 'FontSize', 12);

    % Titolo del grafico
    title(titolo, 'FontSize', 14);

    % Personalizzazione della griglia
    grid on;
    ax = gca; % Ottieni l'asse corrente
    ax.XGrid = 'off'; % Disattiva griglia verticale
    ax.YGrid = 'on'; % Mantieni griglia orizzontale

    % Estetica generale
    set(gca, 'FontSize', 10);
    box off; % Rimuove la cornice del grafico
end
