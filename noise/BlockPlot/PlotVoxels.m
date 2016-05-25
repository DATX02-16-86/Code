function [] = PlotVoxels( filename )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

f = csvread(filename); 

s = size(f); 
n = s(2); 
m = s(1); 
h = m/n; 
 
 
A = permute(reshape(f, n, h, n), [1 3 2]); 
 
clear f 
maxInd = max(max(max(A)));
%colors = [[1,0.76,0.59];[1,0.51,0.51];[1,1,0.66];[0.5,0.5,0.5]]; 

% islands, grass, earth, snow, rock
colors = [[0.75,0.26,0.11];[0.32,0.8,0.33];[0.5,0.37,0.25];[0.95,0.95,0.95];[0.41,0.37,0.47]];

%A = A(:,:,1:20);
for i = 1:maxInd 
    B = A==i; 
    if max(max(max(B))) > 0 
        blockPlot(B, [0 0 0], 'facecolor',colors(i,:),'edgealpha',0.3) 
    end 
end 
axis equal 
axis off

end

