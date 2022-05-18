clc; close all; clear all;

reference_resistances = [100, 1e3, 10e3, 100e3, 1e6, 10e6];

for i = 1:length(reference_resistances)
    [voltage_values, adc_values, res_values] = calculate_values(reference_resistances(i), 10);
    figure;
    plot(voltage_values(1:end-28), res_values(1:end-28), '.');
    xlabel('Vo (V)');
    ylabel('R (Ohm)');
    legend(['Reference Resistance(', int2str(i), '): ', int2str(reference_resistances(i)), ' Ohm']);
end
