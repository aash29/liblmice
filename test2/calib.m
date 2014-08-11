S = load('mouse.log', '-ascii');
readx(:,1)=S(1:2:end,2,:);
readx(:,2)=S(2:2:end,2,:);

ready(:,1)=S(1:2:end,3,:);
ready(:,2)=S(2:2:end,3,:);

dist1=sqrt(readx(:,1).^2 + ready(:,1).^2)
dist2=sqrt(readx(:,2).^2 + ready(:,2).^2)

[m1,i1] = max(dist1)
[m2,i2] = max(dist2)

scale1=max(m1,m2)/m1
scale2=max(m1,m2)/m2

angle1 = atan2(ready(i1,1), readx(i1,1))
angle2 = atan2(ready(i2,2), readx(i2,2))

realx(:,1)=readx(:,1)*cos(angle1).*scale1+ready(:,1)*sin(angle1)*scale1;
realy(:,1)=readx(:,1)*cos(angle1+pi/2)*scale1+ready(:,1)*sin(angle1+pi/2)*scale1;

realx(:,2)=readx(:,2).*cos(angle2)*scale2+ready(:,2)*sin(angle2)*scale2;
realy(:,2)=readx(:,2).*cos(angle2+pi/2)*scale2+ready(:,2)*sin(angle2+pi/2)*scale2;
