// #include <QApplication>
// #include <QGridLayout>
// #include <QHBoxLayout>
// #include <QVBoxLayout>
// #include <QWidget>

// #include "time_functions.hpp"
// #include "rtp_receiver.hpp"

// using namespace std::chrono;

// int main(int argc, char* argv[]) {
//   QApplication app(argc, argv);

//   int main(int argc, char* argv[]) {
//   QApplication app(argc, argv);

// //   // Generate image buffers and their corresponding generators
// //   std::string path1 = "../../../../assets/remote_control_sample_images/view1/";
// //   std::vector<cv::Mat> imageBuffers1 = read_images(path1, cv::Size(256, 256));
// //   ImageIterator image_iterator1(imageBuffers1);
// //   std::cout << "Number of images 1: " << imageBuffers1.size() << std::endl;

// //   std::string path2 = "../../../../assets/remote_control_sample_images/view2/";
// //   std::vector<cv::Mat> imageBuffers2 = read_images(path2, cv::Size(256, 256));
// //   ImageIterator image_iterator2(imageBuffers2);
// //   std::cout << "Number of images 2: " << imageBuffers2.size() << std::endl;

// //   std::string path3 = "../../../../assets/remote_control_sample_images/view3/";
// //   std::vector<cv::Mat> imageBuffers3 = read_images(path3, cv::Size(256, 256));
// //   ImageIterator image_iterator3(imageBuffers3);
// //   std::cout << "Number of images 3: " << imageBuffers3.size() << std::endl;

//   QWidget window;
//   QVBoxLayout vbox(&window);
//   QHBoxLayout hbox1;
//   QHBoxLayout hbox2;

//   ImageUpdatingApp* player1 =
//       new ImageUpdatingApp(image_iterator1, "Source1", &window);
//   ImageUpdatingApp* player2 =
//       new ImageUpdatingApp(image_iterator2, "Source2", &window);
//   ImageUpdatingApp* player3 =
//       new ImageUpdatingApp(image_iterator3, "Source3", &window);
//   hbox1.addWidget(player1);
//   hbox1.addWidget(player2);
//   hbox2.addWidget(player3);
//   vbox.addLayout(&hbox1);
//   vbox.addLayout(&hbox2);
//   window.setLayout(&vbox);
//   window.show();

//   return app.exec();
// }

//   QWidget window;
//   QVBoxLayout vbox(&window);
//   QHBoxLayout hbox1;
//   QHBoxLayout hbox2;

//   ImageUpdatingApp* player1 =
//       new ImageUpdatingApp(image_iterator1, "Source1", &window);
//   ImageUpdatingApp* player2 =
//       new ImageUpdatingApp(image_iterator2, "Source2", &window);
//   ImageUpdatingApp* player3 =
//       new ImageUpdatingApp(image_iterator3, "Source3", &window);
//   hbox1.addWidget(player1);
//   hbox1.addWidget(player2);
//   hbox2.addWidget(player3);
//   vbox.addLayout(&hbox1);
//   vbox.addLayout(&hbox2);
//   window.setLayout(&vbox);
//   window.show();

//   return app.exec();
// }