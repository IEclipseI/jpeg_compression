qf=30
dir=files/jpeg$qf
for filename in $dir/*; do
  echo $filename

  file=$(basename -- "$filename")
#  ./bin/coder $dir/$file tmp/$file.coded
  ./bin/decoder tmp/$file.coded tmp/$file.decoded
  echo "$(diff $dir/$file tmp/$file.decoded)"
done

kotlin ./bin/stat.kts 30

qf=80
dir=files/jpeg$qf
for filename in $dir/*; do
  echo $filename

  file=$(basename -- "$filename")
#  ./bin/coder $dir/$file tmp/$file.coded
  ./bin/decoder tmp/$file.coded tmp/$file.decoded
  echo "$(diff $dir/$file tmp/$file.decoded)"
done

kotlin ./bin/stat.kts $qf