% Load the samples
signal = load_sc16q11('samples.sc16q11');

% Plot the magnitude of the signal in the time domain
plot(abs(signal));

%for reference:
    %samplrate = 40M
    %BW = 28M
    %center frequency = 1762.5M
    %target frequency for antenna RPM = 9.6MHz
    