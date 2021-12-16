clc;
clear all;

fnamein = 'ITMO_background.ppm';
fnameout = 'ITMO_background1.raw';

fID = fopen(fnamein,'rb');
X = fread(fID,'uint8');
size(X)
fclose(fID);

X(1:16)=[];

fID = fopen(fnameout,'wb');
fwrite(fID,X,'uint8');
fclose(fID);

