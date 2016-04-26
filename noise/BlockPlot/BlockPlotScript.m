figure 
f = csvread('D:\projektarbete\code\noise\points3D.txt'); 

s = size(f); 
n = s(2); 
m = s(1); 
h = m/n; 
 
 
A = permute(reshape(f, n, h, n), [1 3 2]); 
 
 
clear f 
maxInd = max(max(max(A))); 
colors = ['r','g','b','k']; 

for i = 1:maxInd 
    B = A==i; 
    if max(max(max(B))) > 0 
        blockPlot(B, [0 0 0])%, 'facecolor',colors(i)) 
    end 
end 
axis equal 
