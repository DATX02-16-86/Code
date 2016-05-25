f = csvread('C:\Users\Adam\Programming\Kandidat\Code\noise\points.txt');
n = length(f) - 1;

f = f(:, 1:n);

%fig = pcolor(f);
surf(f)
%colormap(gray);
%shading flat
%shading interp

axis off
axis equal