The goal of this project is to automatically produce an SGF game record from an image stream.


<center>
<img src="raw/master/goodsquares.jpg" alt="1">
</center>


"kifu" is the Japanese go term for game record. http://senseis.xmp.net/?GameRecord

This software is licenced under the GNU General Public License (GPL)

OpenCV is awesome!!


To find all the points in a completely automated fashion, here's my recipe:

1. find all the closed polygons with 4 sides. (this step is taken from the OpenCV example code)
2. discard all polygons whose area is more or less than 2 std deviations from the median area
3. Compute the convex hull of the set of points of the remaining quadrilaterals
4. find the four corners by discarding points in the convex hull that form a straight line. (this step needs some cleaning up)
5. compute a homography matrix (H) using the four corner points that correspond to the four corners of an ideal go board.
6. use H to transform all the vertices of the ideal go board into the image coordinates
7. Profit!


If you find this program useful, please shoot me an email at ludflu@gmail.com

