/*
 * MainWindow.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: twiemann
 */

#include "MainWindow.hpp"

#include <kfusion/LVRPipeline.hpp>

	void storePicPose(KinFu& kinfu, Affine3f pose, cv::Mat image)
  {
		ImgPose* imgpose = new ImgPose();
		imgpose->pose = pose;
		imgpose->image = image;//.clone();
		//intrinsics wrong? changed rows and cols + /2
		//kinfu.params().cols/2 - 0.5f
		//kinfu.params().rows/2 - 0.5f
		cv::Mat intrinsics = (cv::Mat_<float>(3,3) << kinfu.params().intr.fx*2, 0, 1280/2-0.5f + 3,
												  0, kinfu.params().intr.fx*2, 1024/2-0.5f,
												  0, 0, 1);
		imgpose->intrinsics = intrinsics;
		kinfu.cyclical().addImgPose(imgpose);
	}

MainWindow::MainWindow(QMainWindow* parent) : QMainWindow(parent)
{
	setupUi(this);

	// Create Kinfu object
	KinFuParams params = KinFuParams::default_params();
	m_kinfu = KinFu::Ptr( new KinFu(params) );

	// Setup OpenNI Device
	m_openNISource = new OpenNISource;
	string device = "0";
	if(device.find_first_not_of("0123456789") == std::string::npos)
	{
		cuda::setDevice (atoi(device.c_str()));
		cuda::printShortCudaDeviceInfo (atoi(device.c_str()));

		if(cuda::checkIfPreFermiGPU(atoi(device.c_str())))
		{
			std::cout << std::endl << "Kinfu does not support pre-Fermi GPU architectures, and is not built for them by default. Exiting..." << std::endl;
		}
		m_openNISource->open(atoi(device.c_str()));
	}
	else
	{
		m_openNISource->open(device);
		m_openNISource->triggerPause();
	}
	m_openNISource->setRegistration(true);

	// Generate timer for GPU polling
	m_timer = new QTimer(this);
	m_timer->setInterval(0);

	// Connect signals and slots
	connect(m_pbStart, SIGNAL(pressed()), m_timer, SLOT(start()));
	connect(m_timer, SIGNAL(timeout()), this, SLOT(pollGPUData()));
	connect(m_pbStop, SIGNAL(pressed()), this, SLOT(finalizeMesh()));

    m_meshThread = new MeshUpdateThread(m_kinfu);
    m_meshThread->start();

}

void  MainWindow::finalizeMesh()
{
	m_kinfu->performLastScan();
}

void  MainWindow::setupVTK()
{
	// Grab relevant entities from the qvtk widget
	m_renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = this->qvtkWidget->GetRenderWindow();

	m_renderWindowInteractor = this->qvtkWidget->GetInteractor();
	m_renderWindowInteractor->Initialize();

	// Finalize QVTK setup by adding the renderer to the window
	renderWindow->AddRenderer(m_renderer);
}

void MainWindow::pollGPUData()
{
	KinFu& kinfu = *m_kinfu;
	cv::Mat depth, image, image_copy;
	static int has_image = 0;
	static int image_count = 0;
	static int frame_count = 0;

   static std::vector<Affine3f> posen;
		static std::vector<cv::Mat> rvecs;

		static Affine3f best_pose;
		static cv::Mat best_rvec,best_image;
		static float best_dist=0.0;

	if(!(m_kinfu->hasShifted() && m_kinfu->isLastScan()))
	{
		int has_frame = m_openNISource->grab(depth, image);
		cv::flip(depth, depth, 1);
		cv::flip(image, image, 1);

		if (has_frame == 0)
		{
			std::cout << "Can't grab" << std::endl;
			return;
		}

		// Check if oni file ended
		if (has_frame == 2)
		{
			m_kinfu->performLastScan();
		}
		m_depth_device.upload(depth.data, depth.step, depth.rows, depth.cols);
		has_image = kinfu(m_depth_device);
		if(has_image) frame_count++;
	}


	if (!(m_kinfu->hasShifted() && m_kinfu->isLastScan()) && has_image)
            {
				//biggest rvec difference -> new pic
				//
				double ref_timer = cv::getTickCount();
				
				if(rvecs.size()<1){
					image.copyTo(image_copy);

					//buffer of all imgposes
					rvecs.push_back(cv::Mat(kinfu.getCameraPose().rvec()));
					posen.push_back(kinfu.getCameraPose());

					//storePicPose(kinfu, image_copy);
					//extractImage(kinfu, image_copy);
				}
                else
                {
					float dist = 0.0;
					cv::Mat mom_rvec(kinfu.getCameraPose().rvec());
					for(size_t z=0;z<rvecs.size();z++){
						dist += norm(mom_rvec-rvecs[z]);
					}
					if(dist > best_dist){
						best_dist = dist;
						//mom_rvec.copyTo(best_rvec);
						//image.copyTo(best_image);
						best_rvec = mom_rvec.clone();
						best_image = image.clone();
						best_pose = kinfu.getCameraPose();
						//std::cout << "better image found, sum rvec distances: " << best_dist << std::endl;
					}
					//if(time - 3.0 > 0)
					if(true && (frame_count % 7 == 0))
					{
					  cout  <<"STORE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

						rvecs.push_back(best_rvec);
						posen.push_back(best_pose);

						storePicPose(*m_kinfu, best_pose, best_image);
						//extractImage(kinfu, best_image);
						sample_poses_.push_back(m_kinfu->getCameraPose());
						 std::cout << "image taken "<< image_count++ << ", time: "<< time << std::endl;
		

					}
				}
            }




    const int mode = 4;

    // Raycast image and download from device
    m_kinfu->renderImage(m_viewImage, mode);
    m_deviceImg.create(m_viewImage.rows(), m_viewImage.cols(), CV_8UC4);
    m_viewImage.download(m_deviceImg.ptr<void>(), m_deviceImg.step);

    // Convert cv mat to pixmap and render into label
    m_displayRaycastLabel->setPixmap(
    		QPixmap::fromImage(
    				QImage((unsigned char*) m_deviceImg.data,
    				m_deviceImg.cols,
					m_deviceImg.rows,
					QImage::Format_RGB32)));

    m_displayImageLabel->setPixmap(
    		QPixmap::fromImage(
    				QImage((unsigned char*) image.data,
    				image.cols,
					image.rows,
					QImage::Format_RGB888).rgbSwapped()));


}





MainWindow::~MainWindow()
{
	if(m_timer)
	{
		delete m_timer;
	}

	if(m_openNISource)
	{
		delete m_openNISource;
	}

	m_kinfu.release();

	m_meshThread->quit();
	m_meshThread->wait();
	delete m_meshThread;
}

