clc;
clear all;

fname = 'fruits.ppm';
X = imread(fname);

figure(1)
imshow([X]);
drawnow;