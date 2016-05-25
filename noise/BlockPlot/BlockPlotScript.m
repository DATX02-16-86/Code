figure 
f = csvread('C:\Users\Adam\Programming\Kandidat\Code\noise\points3D_4.txt'); 

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

%% test
clear figure
PlotVoxels('C:\Users\Adam\Programming\Kandidat\Code\noise\points3D_1.txt')

%% rotating
for i = 0:3
    clf
    PlotVoxels(['C:\Users\Adam\Programming\Kandidat\Code\noise\points3D_', num2str(i+1),'.txt'])
    for n = 0:10:90
       view([-135 + i*90+n, 20])
       drawnow   
    end
end
%% gif 

filename = 'test.gif'

for n = 1:1:360
    camorbit(1,0)
    drawnow
    frame = getframe(gcf);
    im = frame2im(frame);
    [A,map] = rgb2ind(im,256); 
    if n == 1;
        imwrite(A,map,filename,'gif','LoopCount',Inf,'DelayTime',1);
    else
        imwrite(A,map,filename,'gif','WriteMode','append','DelayTime',1);
    end
    disp(n)
end

disp('done')

%% supergif
filename = 'megagif.gif'

for i = 0:3
    clf
    PlotVoxels(['C:\Users\Adam\Programming\Kandidat\Code\noise\points3D_', num2str(i+1),'.txt'])
    for n = 1:1:180
        view([-135 + i*180+n, 20])
        drawnow
        frame = getframe(gcf);
        im = frame2im(frame);
        [A,map] = rgb2ind(im,256); 
        if n == 1 && i == 0;
            imwrite(A,map,filename,'gif','LoopCount',Inf,'DelayTime',0.05);
        else
            imwrite(A,map,filename,'gif','WriteMode','append','DelayTime',0.05);
        end
    end
    disp(['done with', num2str(i)])
end
disp('done')