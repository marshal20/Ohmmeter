function [voltage_values, adc_values, res_values] = calculate_values(reference_resistance, adc_bits_count)

adc_max = 2^adc_bits_count;

adc_values = 0:(adc_max-1);
voltage_values = adc_values/adc_max*5;
res_values = reference_resistance ./ (1./(adc_values/adc_max) - 1);
