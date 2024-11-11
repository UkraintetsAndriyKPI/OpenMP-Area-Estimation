#include <math.h>
#include <time.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_TRIANGLES 3
#define MAX_CIRCLES 3
#define MAX_TRIANGLE_SIDE 4
#define MAX_CIRCLE_R 4

// Point struct
typedef struct {
    double x;
    double y;
} Point;

// Rectangle, Triangle and Circle structs
typedef struct {
    double width;
    double height;
} Rectangle;

typedef struct {
    Point center;
    double side;
    double angle;  // Angle in degrees
} Triangle;

typedef struct {
    Point center;
    double radius;
} Circle;

// Function for checking if Point in Circle
int isInsideCircle(Point p, Circle circle) {
    // length between point and Circle.center
    double dx = p.x - circle.center.x;
    double dy = p.y - circle.center.y;
    // (x−xc)^2 + (y−yc)^2 <= r^2
    return pow(dx, 2) + pow(dy, 2) <= pow(circle.radius, 2);
}

// Function for checking if Point in Triangle
int isInsideTriangle(Point p, Triangle triangle) {
    double h = (sqrt(3) / 2) * triangle.side;  // triangle height

    //      C
    //      ^
    //     / \
    //    /   \
    //   /  .  \
    //  /       \
    // A _ _ _ _ B
    // Length between Points(A, B, C) and Triangle.center [BEFORE ROTATING]
    double dxA = -triangle.side / 2, dyA = -h / 3;
    double dxB = triangle.side / 2, dyB = -h / 3;
    double dxC = 0, dyC = 2 * (h / 3);

    // Convert angle from degrees to radians
    double angle_rad = triangle.angle * (M_PI / 180.0);

    // cos and sin of rotate Angle
    double cosOfAngle = cos(angle_rad);
    double sinOfAngle = sin(angle_rad);

    // Rotating all vertices by triangle.angle
    double xA = triangle.center.x + dxA * cosOfAngle - dyA * sinOfAngle;
    double yA = triangle.center.y + dxA * sinOfAngle + dyA * cosOfAngle;
    double xB = triangle.center.x + dxB * cosOfAngle - dyB * sinOfAngle;
    double yB = triangle.center.y + dxB * sinOfAngle + dyB * cosOfAngle;
    double xC = triangle.center.x + dxC * cosOfAngle - dyC * sinOfAngle;
    double yC = triangle.center.y + dxC * sinOfAngle + dyC * cosOfAngle;

    double dA_B = (p.x - xA) * (yB - yA) - (p.y - yA) * (xB - xA);
    double dB_C = (p.x - xB) * (yC - yB) - (p.y - yB) * (xC - xB);
    double dC_A = (p.x - xC) * (yA - yC) - (p.y - yC) * (xA - xC);

    // If as a result both values ​​are true (has_negative and
    // has_positive), this means that the point is on different sides of the
    // triangle lines, that is, the point is outside the triangle.
    int has_negative = (dA_B < 0) || (dB_C < 0) || (dC_A < 0);
    int has_positive = (dA_B > 0) || (dB_C > 0) || (dC_A > 0);
    return !(has_negative && has_positive);
}

double calculateArea(Rectangle rectangle, Circle* circles, int circlesAmount,
                     Triangle* triangles, int trianglesAmount, int points_amount) {
    double rectangle_area = rectangle.width * rectangle.height;
    int count_safe = 0;

    #pragma omp parallel
    {
        unsigned int seed = omp_get_thread_num();
        #pragma omp for reduction(+ : count_safe)
        for (int i = 0; i < points_amount; i++) {
            Point rand_p;  // Random point
            rand_p.x = ((double)rand_r(&seed) / RAND_MAX) * rectangle.width;
            rand_p.y = ((double)rand_r(&seed) / RAND_MAX) * rectangle.height;

            int inside_any_shape = 0;

            // Checking if point in Circles
            for (int j = 0; j < circlesAmount; j++) {
                if (isInsideCircle(rand_p, circles[j])) {
                    inside_any_shape = 1;
                    break;
                }
            }

            // If point is not in circles
            if (!inside_any_shape) {
                for (int j = 0; j < trianglesAmount; j++) {
                    if (isInsideTriangle(rand_p, triangles[j])) {
                        inside_any_shape = 1;
                        break;
                    }
                }
            }

            if (!inside_any_shape) {
                count_safe++;
            }
        }
    }
    return rectangle_area * ((double)count_safe / points_amount);
}

int main(int argc, char** argv) {
    srand(time(NULL));
    int points[] = {100, 10000, 1000000, 10000000};

    int circlesAmount = rand() % MAX_CIRCLES + 1;
    int trianglesAmount = rand() % MAX_TRIANGLES + 1;

    // Figures declaring
    Rectangle rectangle = {15.0, 20.0};
    Circle* circles = (Circle*)malloc(circlesAmount * sizeof(Circle));
    Triangle* triangles = (Triangle*)malloc(trianglesAmount * sizeof(Triangle));

    printf("\tAll created figures.\n");
    printf("Ractangle : width = %.2f, height = %.2f\n", rectangle.width, rectangle.height);
    // Circles generating
    for (int i = 0; i < circlesAmount; i++) {
        circles[i].center.x = ((double)rand() / RAND_MAX) * rectangle.width;
        circles[i].center.y = ((double)rand() / RAND_MAX) * rectangle.height;
        circles[i].radius = ((double)rand() / RAND_MAX) * MAX_CIRCLE_R;
        printf("Circle %d: center(%.2f, %.2f), radius = %.2f\n",
           i, circles[i].center.x, circles[i].center.y, circles[i].radius);
    }

    // Triangles generating
    for (int i = 0; i < trianglesAmount; i++) {
        triangles[i].center.x = ((double)rand() / RAND_MAX) * rectangle.width;
        triangles[i].center.y = ((double)rand() / RAND_MAX) * rectangle.height;
        triangles[i].side = ((double)rand() / RAND_MAX) * MAX_TRIANGLE_SIDE;
        triangles[i].angle = ((double)rand() / RAND_MAX) * 360;
        printf("Triangle %d: center(%.2f, %.2f), side = %.2f, angle = %.2f\u00B0\n",
           i + 1, triangles[i].center.x, triangles[i].center.y,
           triangles[i].side, triangles[i].angle);
    }

    printf("\n\tArea calculation.\n");
    for (int i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
        int num_points = points[i];
        double estimated_area =
            calculateArea(rectangle, circles, circlesAmount, triangles,
                          trianglesAmount, num_points);
        printf("Кількість точок: %d, Оцінка площі: %f\n", num_points,
               estimated_area);
    }

    return 0;
}
