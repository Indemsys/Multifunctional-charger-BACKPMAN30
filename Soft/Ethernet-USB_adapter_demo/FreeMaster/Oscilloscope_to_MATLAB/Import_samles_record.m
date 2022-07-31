opts = delimitedTextImportOptions("NumVariables", 2);

% Specify range and delimiter
opts.DataLines = [4, Inf];
opts.Delimiter = "\t";

% Specify column names and types
opts.VariableNames = ["Seconds", "samples"];
opts.VariableTypes = ["double", "double"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Import the data
oscilloscope = readtable("oscilloscope.txt", opts);


%% Clear temporary variables
clear opts

histogram(oscilloscope.samples)


m = mean(oscilloscope.samples);
dev = std(oscilloscope.samples);
s_min = min(oscilloscope.samples);
s_max = max(oscilloscope.samples);
RANGE = 2^16;
VOLTAGE = 3.3;

arr_size = numel(oscilloscope.samples);
delta = s_max- s_min;
max_delta = delta*VOLTAGE/RANGE;
max_dev = dev*VOLTAGE/RANGE;

fmt = 'Average= %f , delta = %f, deviation=%f, samples count=%d';
str = sprintf(fmt,m,delta,dev,arr_size)



