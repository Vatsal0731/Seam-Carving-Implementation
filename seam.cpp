#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

vector<int> findseam_horizontal(Mat energy) {
    int cols = energy.cols;
    vector<int> temp(cols);
    int rows = energy.rows;
    
    vector<vector<double>> dp(rows, vector<double>(cols, 0.0));
    vector<vector<int>> path(rows, vector<int>(cols, 0));

        int x = 0 ;
         while (x < rows) {
             dp[x][0] = energy.at<double>(x, 0);
             x++;
        }

        int y=1 ;
        while(y < cols) {

        for (int i = 0; i < rows; i++) {

            int index_min = i;
            double Energy_min = dp[i][y - 1];

            if (i < rows - 1 && dp[i + 1][y - 1] < Energy_min) {
                index_min = i + 1;
                Energy_min = dp[i + 1][y - 1];
                
            }
            if (i > 0 && dp[i - 1][y - 1] < Energy_min) {
                index_min = i - 1;
                Energy_min = dp[i - 1][y - 1];
                
            }
            path[i][y] = index_min ; 
            dp[i][y] = energy.at<double>(i, y) + Energy_min ;
        }
        y++ ;
    }

    int index_min = 0;
    double Energy_min = DBL_MAX;
    
    x = 0 ;

    while(x < rows) {
        if (dp[x][cols - 1] < Energy_min) {
            Energy_min = dp[x][cols - 1];
            index_min = x;
        }
        x++ ;
    }

    temp[cols - 1] = index_min;
    for (int k = cols - 2; k >= 0; k--) {
        temp[k] = path[temp[k + 1]][k + 1];
    }

    return temp ;
}




vector<int> findseam_vertical(Mat energy) {
    int rows = energy.rows;
    vector<int> temp(rows);
    int cols = energy.cols;
    
    vector<vector<double>> dp(rows, vector<double>(cols, 0.0));
    vector<vector<int>> path(rows, vector<int>(cols, 0));


   
    int x  = 0;
    while( x < cols) {
        dp[0][x] = energy.at<double>(0, x);
        x++ ;
    }
    
    int y =1 ;
    while( y < rows ) {
        for (int j = 0; j < cols; j++) {
            int index_min = j;
            double Energy_min = dp[y - 1][j];
            

            if ( dp[y - 1][j + 1] < Energy_min && j < cols - 1 ) {
                index_min = j + 1;
                Energy_min = dp[y - 1][j + 1];
            }

            if ( dp[y - 1][j - 1] < Energy_min && j > 0 ) {
                index_min = j - 1;
                Energy_min = dp[y - 1][j - 1];
            }
            
            path[y][j] = index_min;
            dp[y][j] = energy.at<double>(y, j) + Energy_min;
            
        }
        y++ ;
    }

    
    double Energy_min = DBL_MAX;
    int index_min = 0;

    x =0 ;
    while( x < cols ) {
        if (dp[rows - 1][x] < Energy_min) {
            Energy_min = dp[rows - 1][x];
            index_min = x;
        }
        x++ ;
    }

    temp[rows - 1] = index_min;

    for (int i = rows - 2; i >= 0; i--) {
        temp[i] = path[i + 1][temp[i + 1]];
    }

    return temp;
}


Mat removeSeam(bool vimg, Mat& img, vector<int> seam) {

    Mat output;
    int numCols = img.cols;
    int numRows = img.rows;

   

    if (!vimg) {

        output = Mat(numRows - 1, numCols, CV_8UC3);

        for (int col = 0; col < numCols; col++) {

            int seamRow = seam[col] ;

            for (int row = 0; row < seamRow; row++) {
                Vec3b pxl = img.at<Vec3b>(row, col);
                output.at<Vec3b>(row, col) = pxl;
            }

            for (int row = seamRow + 1; row < numRows; row++) {
                Vec3b pxl = img.at<Vec3b>(row, col);
                output.at<Vec3b>(row - 1, col) = pxl;
            }

        }
    } 
    else {
        output = Mat(numRows, numCols - 1, CV_8UC3);

        for (int row = 0; row < numRows; row++) {
            int seamCol = seam[row];

            for (int col = 0; col < seamCol; col++) {
                Vec3b pxl =  img.at<Vec3b>(row, col);
                output.at<Vec3b>(row, col) = pxl;
            }

            for (int col = seamCol + 1; col < numCols; col++) {
                Vec3b pixel = img.at<Vec3b>(row, col);
                output.at<Vec3b>(row, col - 1) = pixel;
            }
        }
    }

    return output;
}

Mat getEnergyImage(Mat& image) {


    int ddepth = CV_16S;
    Mat abs_slope_x, abs_slope_y , slope , energyImage , slope_x  , slope_y , blur_img, grey_img;
    GaussianBlur(image, blur_img, Size(3, 3), 0, 0, BORDER_DEFAULT);
    cvtColor(blur_img, grey_img, COLOR_BGR2GRAY);
    Scharr(grey_img, slope_x, ddepth, 1, 0);
    Scharr(grey_img, slope_y, ddepth, 0, 1);
    convertScaleAbs(slope_x, abs_slope_x);
    convertScaleAbs(slope_y, abs_slope_y);
    addWeighted(abs_slope_x, 0.5, abs_slope_y, 0.5, 0, slope);
    slope.convertTo(energyImage, CV_64F, 1.0 / 255.0);

    return energyImage;
}

int main(int argc , char** argv) {

    if(argc!=4){
        cout<<"invalid parameters"<<endl ;
        return -1;
    }
    string img_path = argv[1] ;
    int n = stoi(argv[2]) ;
    int m = stoi(argv[3]) ;
     
    Mat image = imread( img_path, IMREAD_COLOR);

    if (image.empty()) {
        cout << "image no found" << endl;
        return -1;
    }

    int imgc = image.cols ;
    int imgr = image.rows ;

    if(imgc<n){
        cout<<"width restriction"<<endl;
        return -1 ;
    }
    if(imgr<m){
        cout<<"height restriction"<<endl;
        return -1 ;
    }
   
    while (imgc > 1 && n) {
        n-- ;
        Mat energy = getEnergyImage(image);
        vector<int> seam ; 
        seam = findseam_vertical(energy) ; 
        image = removeSeam(true , image, seam );
    }

    while (imgr > 1 && m) {
        m-- ;
        Mat energy = getEnergyImage(image);
        vector<int> seam ;
        seam  = findseam_horizontal(energy);
        image = removeSeam( false , image, seam );
    }

    imwrite("output.jpeg", image);
    return 0;
}