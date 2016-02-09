typedef struct {
  double x;
  double y;
} Point;

typedef struct {
  double k;
  double m;
} Line;

Point randomPoint(double xmax, double ymax);
Line bisector(Point a, Point b);
