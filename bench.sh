while true;
do
docker run --rm --volumes-from data cloudsuite/in-memory-analytics /data/ml-latest-small /data/myratings.csv
done
