/*
C++ version of ISBF for TraceBounds
*/
#include "isbfcpp.h"

#include <iostream>
#include <list>
#include <cmath>
#include <vector>

isbfcpp::isbfcpp()
{
}

void isbfcpp::rot90(int nrows, int ncols, std::vector <std::vector<int> > matrix,
                    std::vector <std::vector<int> > &matrix270,
                    std::vector <std::vector<int> > &matrix180,
                    std::vector <std::vector<int> > &matrix90)
{
    // 0 to 270 degree
    for (int i=0; i<nrows; i++){
      for (int j=0;j<ncols; j++){
        matrix270[j][nrows-1-i] = matrix[i][j];
      }
    }
    // 270 to 180 degree
    for (int i=0; i<ncols; i++){
      for (int j=0;j<nrows; j++){
        matrix180[j][ncols-1-i] = matrix270[i][j];
      }
    }
    // 180 to 90 degree
    for (int i=0; i<nrows; i++){
      for (int j=0;j<ncols; j++){
        matrix90[j][nrows-1-i] = matrix180[i][j];
      }
    }
}

std::vector <std::vector<int> > isbfcpp::traceBoundary(int nrows, int ncols, std::vector <std::vector<int> > mask, int startX, int startY, float inf)
{
    // initialize boundary vector
    std::list<int> boundary_listX;
    std::list<int> boundary_listY;

    // push the first x and y points
    boundary_listX.push_back(startX);
    boundary_listY.push_back(startY);

    // initialize matrix for 90, 180, 270 degrees
    std::vector <std::vector<int> > matrix90(ncols, std::vector<int>(nrows));
    std::vector <std::vector<int> > matrix180(nrows, std::vector<int>(ncols));
    std::vector <std::vector<int> > matrix270(ncols, std::vector<int>(nrows));

    // rotate matrix for 90, 180, 270 degrees
    rot90(nrows, ncols, mask, matrix270, matrix180, matrix90);

    // set defalut direction
    int DX = 1;
    int DY = 0;

    // set the number of rows and cols for ISBF
    int rowISBF = 3;
    int colISBF = 2;

    float angle;

    // set size of X: the size of X is equal to the size of Y
    int sizeofX;

    // loop until true
    while(1) {

      std::vector <std::vector<int> > h(rowISBF, std::vector<int>(colISBF));

      // initialize a and b which are indices of ISBF
      int a = 0;
      int b = 0;

      int x = boundary_listX.back();
      int y = boundary_listY.back();

      if((DX == 1)&&(DY == 0)){
        for (int i = ncols-x-2; i < ncols-x+1; i++) {
          for (int j = y-1; j < y+1; j++) {
              h[a][b] = matrix90[i][j];
              b++;
          }
          b = 0;
          a++;
        }
        angle = M_PI/2;
      }

      else if((DX == 0)&&(DY == -1)){
        for (int i = y-1; i < y+2; i++) {
          for (int j = x-1; j < x+1; j++) {
              h[a][b] = mask[i][j];
              b++;
          }
          b = 0;
          a++;
        }
        angle = 0;
      }

      else if((DX == -1)&&(DY == 0)){
        for (int i = x-1; i < x+2; i++) {
          for (int j = nrows-y-2; j < nrows-y; j++) {
              h[a][b] = matrix270[i][j];
              b++;
          }
          b = 0;
          a++;
        }
        angle = 3*M_PI/2;
      }

      else{
        for (int i = nrows-y-2; i < nrows-y+1; i++) {
          for (int j = ncols-x-2; j < ncols-x; j++) {
              h[a][b] = matrix180[i][j];
              b++;
          }
          b = 0;
          a++;
        }
        angle = M_PI;
      }

      // initialize cX and cY which indicate directions for each ISBF
      std::vector<int> cX(1);
      std::vector<int> cY(1);

      if (h[1][0] == 1) {
          // 'left' neighbor
          cX[0] = -1;
          cY[0] = 0;
          DX = -1;
          DY = 0;
      }
      else{
          if((h[2][0] == 1)&&(h[2][1] != 1)){
              // inner-outer corner at left-rear
              cX[0] = -1;
              cY[0] = 1;
              DX = 0;
              DY = 1;
          }
          else{
              if(h[0][0] == 1){
                  if(h[0][1] == 1){
                      // inner corner at front
                      cX[0] = 0;
                      cY[0] = -1;
                      cX.push_back(-1);
                      cY.push_back(0);
                      DX = 0;
                      DY = -1;
                  }
                  else{
                      // inner-outer corner at front-left
                      cX[0] = -1;
                      cY[0] = -1;
                      DX = 0;
                      DY = -1;
                  }
              }
              else if(h[0][1] == 1){
                // front neighbor
                cX[0] = 0;
                cY[0] = -1;
                DX = 1;
                DY = 0;
              }
              else{
                // outer corner
                DX = 0;
                DY = 1;
              }
          }
      }

      // transform points by incoming directions and add to contours
      float s = sin(angle);
      float c = cos(angle);

      if(!((cX[0]==0)&&(cY[0]==0))){
          for(int t=0; t< int(cX.size()); t++){
              float a, b;
              int cx = cX[t];
              int cy = cY[t];

              a = c*cx - s*cy;
              b = s*cx + c*cy;

              x = boundary_listX.back();
              y = boundary_listY.back();

              boundary_listX.push_back(x+roundf(a));
              boundary_listY.push_back(y+roundf(b));
          }
      }

      float i, j;
      i = c*DX - s*DY;
      j = s*DX + c*DY;
      DX = roundf(i);
      DY = roundf(j);

      // get length of the current linked list
      sizeofX = boundary_listX.size();

      if (sizeofX > 3) {

        int fx1 = *boundary_listX.begin();
        int fx2 = *std::next(boundary_listX.begin(), 1);
        int fy1 = *boundary_listY.begin();
        int fy2 = *std::next(boundary_listY.begin(), 1);
        int lx1 = *std::prev(boundary_listX.end());
        int ly1 = *std::prev(boundary_listY.end());
        int lx2 = *std::prev(boundary_listX.end(), 2);
        int ly2 = *std::prev(boundary_listY.end(), 2);
        int lx3 = *std::prev(boundary_listX.end(), 3);
        int ly3 = *std::prev(boundary_listY.end(), 3);

        // check if the first and the last x and y are equal
        if ((sizeofX > inf)|| \
        ((lx1 == fx2)&&(lx2 == fx1)&&(ly1 == fy2)&&(ly2 == fy1))){
          // remove the last element
            boundary_listX.pop_back();
            boundary_listY.pop_back();
            break;
        }
        if (int(cX.size()) == 2)
          if ((lx2 == fx2)&&(lx3 == fx1)&&(ly2 == fy2)&&(ly3 == fy1)){
            boundary_listX.pop_back();
            boundary_listY.pop_back();
            boundary_listX.pop_back();
            boundary_listY.pop_back();
            break;
        }
      }
    }

    std::vector <std::vector<int> > boundary(2, std::vector<int>(sizeofX));

    boundary[0].assign(boundary_listX.begin(), boundary_listX.end());
    boundary[1].assign(boundary_listY.begin(), boundary_listY.end());

    return boundary;
}

isbfcpp::~isbfcpp()
{
}