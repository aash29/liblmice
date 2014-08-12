close all
clear all
S = load('mouse.log', '-ascii');

readx(:,1)=S(1:2:end,2,:);
readx(:,2)=S(2:2:end,2,:);

ready(:,1)=S(1:2:end,3,:);
ready(:,2)=S(2:2:end,3,:);

[scale,angle] = calib(readx,ready);


scale=[1;1.0047];
angle=[1.5968;1.5708];

realx(:,1)=readx(:,1)*cos(angle(1)).*scale(1)+ready(:,1)*sin(angle(1))*scale(1);
realy(:,1)=readx(:,1)*cos(angle(1)+pi/2)*scale(1)+ready(:,1)*sin(angle(1)+pi/2)*scale(1);

realx(:,2)=readx(:,2).*cos(angle(2))*scale(2)+ready(:,2)*sin(angle(2))*scale(2);
realy(:,2)=readx(:,2).*cos(angle(2)+pi/2)*scale(2)+ready(:,2)*sin(angle(2)+pi/2)*scale(2);


r1x=800;
r1y=0;
r2x=-800;
r2y=0;

% X=0;
% Y=0;
% Theta=0;

pos=[0 0 0];

for i=1:length(realx)

A=[1 0 -r1y;
   0 1 r1x;
   1 0 -r2y;
   0 1 r2x];
a=[realx(i,1);
    realy(i,1);
    realx(i,2);
    realy(i,2)];

dx=A\a;

Theta=pos(end,3);

T=[cos(Theta(end)) -sin(Theta(end)) 0;
    sin(Theta(end)) cos(Theta(end)) 0;
    0 0 1];

   
% X=[X;X(end)+dx(1)*cos(Theta(end)) - dx(2)];
% Y=[Y;Y(end)+dx(2)];
% Theta=[Theta;Theta(end)+dx(3)];

%[X1]=[X(end);Y(end);Theta(end)]+T*dx;

X1=pos(end,:)'+T*dx;

pos=[pos; X1'];

end
figure
plot(pos(:,1),pos(:,2));
axis equal
figure
plot(pos(:,3));