#include "image_updating_app.hpp"

ImageUpdatingApp::ImageUpdatingApp(const std::string& sdp_path,
                                   const QString& label,
                                   QWidget* parent) :
    QWidget(parent),
    
    display_width_(256),
    display_height_(256),
    num_images_(0),
    start_time_(QDateTime::currentMSecsSinceEpoch()) {
  setWindowTitle(label);

  RTPReceiver* rtp_receiver = new RTPReceiver(sdp_path);
  video_thread_ = new VideoThread(rtp_receiver, parent);

  image_label_ = new QLabel(this);
  image_label_->resize(display_width_, display_height_);
  image_label_->setAlignment(Qt::AlignCenter);

  text_label_ = new QLabel(this);

  // Create a vertical box layout and add the image and text labels
  vbox_layout_ = new QVBoxLayout(this);
  vbox_layout_->addWidget(image_label_);
  vbox_layout_->addWidget(text_label_);

  // Set the vbox layout as the widget's layout
  setLayout(vbox_layout_);

  // Connect the VideoThread's signal to the updateImage slot
  QObject::connect(video_thread_, SIGNAL(changePixmap(const QImage&)), this,
                   SLOT(updateImage(const QImage&)));
  video_thread_->start();
}

ImageUpdatingApp::~ImageUpdatingApp() {
  delete video_thread_;
  delete image_label_;
  delete text_label_;
  delete vbox_layout_;
}

void ImageUpdatingApp::closeEvent(QCloseEvent* event) {
  video_thread_->stop();
  event->accept();
}

void ImageUpdatingApp::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    close();
  }
}

void ImageUpdatingApp::updateImage(const QImage& qImg) {
  image_label_->setPixmap(QPixmap::fromImage(qImg));
  updateTextLabel();
}

void ImageUpdatingApp::updateTextLabel() {
  num_images_++;
  unsigned int time_elapsed = QDateTime::currentMSecsSinceEpoch() - start_time_;
  QString labelText =
      QString(
          "%1\nn_images: %2\nTime elapsed in ms: %3\nFrequency of images: %4")
          .arg(windowTitle())
          .arg(num_images_)
          .arg(static_cast<unsigned int>(time_elapsed))
          .arg(
              static_cast<unsigned int>(num_images_ / (time_elapsed / 1000.0)));
  text_label_->setText(labelText);
}
