#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "image_updating_app.hpp"
#include "rtp_receiver.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  QWidget window;
  QVBoxLayout vbox(&window);
  QHBoxLayout hbox1;

  const std::string sdp_path1(argc > 1 ? argv[1] : "test.sdp");
  std::unique_ptr<ImageUpdatingApp> player1(
      new ImageUpdatingApp(sdp_path1, "Source1", &window));

  hbox1.addWidget(player1.get());
  vbox.addLayout(&hbox1);
  window.setLayout(&vbox);
  window.show();

  return app.exec();
}
