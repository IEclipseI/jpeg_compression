clc;
clear all;

fname = 'fruits.png';
fnamein = 'fruits.raw';
X = imread(fname);

Size = size(X)

B = X(:,:,1);
G = X(:,:,2);
R = X(:,:,3);

B = B';
G = G';
R = R';
B = B(:);
G = G(:);
R = R(:);
Y = zeros(1,3*Size(1)*Size(2));

j=1;
for i=1:length(B)
    Y(j)=B(i);
    j = j + 1;
    Y(j)=G(i);
    j = j + 1;
    Y(j)=R(i);
    j = j + 1;
end


fID = fopen(fnamein,'wb');
fwrite(fID,Y,'uint8');
fclose(fID);

%figure(1)
%imshow([X]);
%drawnow;