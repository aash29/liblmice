S = load('mouse.log', '-ascii');
readx(:,1)=S(1:2:end,2,:);
readx(:,2)=S(2:2:end,2,:);

ready(:,1)=S(1:2:end,3,:);
ready(:,2)=S(2:2:end,3,:);