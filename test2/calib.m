%S = load('mouse.log', '-ascii');
function [scale,angle] = calib(readx,ready);

    dist1=sqrt(readx(:,1).^2 + ready(:,1).^2)
    dist2=sqrt(readx(:,2).^2 + ready(:,2).^2)

    [m1,i1] = max(dist1)
    [m2,i2] = max(dist2)

    scale1=max(m1,m2)/m1
    scale2=max(m1,m2)/m2

    angle1 = atan2(ready(i1,1), readx(i1,1))
    angle2 = atan2(ready(i2,2), readx(i2,2))
    
    scale=[scale1;scale2];
    angle=[angle1;angle2];

end