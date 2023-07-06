#pragma once

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QObject>
#include <QPixmap>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>
#include <opencv2/opencv.hpp>

#include "video_thread.hpp"


class ImageUpdatingApp : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief Construct a new ImageUpdatingApp instance.
   * @param image_iterator Image Iterator which returns one cv::Mat at a time
   * from a vector of Cv::Mat.
   * @param label of the corresponding image-widget
   * */
  explicit ImageUpdatingApp(RTPReceiver& rtp_receiver,
                            const QString& label,
                            QWidget* parent = nullptr);

  ~ImageUpdatingApp();
  /**
   * @brief Explicitly stop the VideoThread before closing to prevent errors.
   * @param event QCloseEvent.
   * */
  void closeEvent(QCloseEvent* event) override;

  /**
   * @brief Close the widget if the user presses the Escape key.
   * @param event QKeyEvent.
   * */
  void keyPressEvent(QKeyEvent* event) override;

private slots:
  /**
   * @brief Paints image received from signal returning image loaded from disk.
   * @param qImg QImage.
   * */
  void updateImage(const QImage& qImg);

private:
  /** label responsible for displaying image */
  QLabel* image_label_;

  /** text label displaying frequency and other debugging information */
  QLabel* text_label_;

  /** parent vertical layout containing all image widgets and any other
   * (joystick controls for example) widgets */
  QVBoxLayout* vbox_layout_;

  /** thread responsible for emitting signal which fires every time an
   * image-loading is complete
   */
  VideoThread* video_thread_;

  /** width of image-wdiget */
  int display_width_;

  /** height of image-wdiget */
  int display_height_;

  /** counter var tracking number of images displayed so far */
  int num_images_;

  /** starting time var to track operating frequency */
  unsigned int start_time_;

  /**
   * @brief Update the text label with the relevant information.
   * */
  void updateTextLabel();
};
