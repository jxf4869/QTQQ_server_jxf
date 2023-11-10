#pragma once

#include <QTcpSocket>

class TcpSocket  : public QTcpSocket
{
	Q_OBJECT

public:
	TcpSocket();
	~TcpSocket();


	void run();


signals:
	//从客户端收到数据发射信号,告诉服务端有数据要处理
	void signalGetDataFromClient(QByteArray&, int);
	//告诉server有客户端断开连接
	void signalCilentDisconnect(int);

private slots:
	//处理readyRead信号读取数据
	void onReceiveData();
	//处理客户端断开连接
	void onClientDisconnect();

private:
	int m_descriptor;//描述符,用于唯一标识

};
