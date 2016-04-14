
figure
f = csvread('C:\Users\Admin\Documents\Code\noise\Noisy\points3D.txt');

s = size(f);
n = s(2);
m = s(1);
h = m/n;

A = permute(reshape(f, n, h, n), [1 3 2]);

clear f

p = blockPlot(A);
axis equal

