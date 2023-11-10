#include "TcpServer.h"
#include "TcpSocket.h"
#include <QTcpSocket>

TcpServer::TcpServer(int port) :m_port(port)
{}

TcpServer::~TcpServer()
{}


bool TcpServer::run()
{
	if (this->listen(QHostAddress::AnyIPv4, m_port)) //监听任何主机地址
	{	
		qDebug() << QString::fromLocal8Bit("服务端监听端口 %1 成功!").arg(m_port);
		return true;
	}
	else 
	{
		qDebug() << QString::fromLocal8Bit("服务端监听端口 %1 失败!").arg(m_port);
		return false;
	}
}
//服务端有新的连接来时,分配一个新的TcpSocket进行管理
void TcpServer::incomingConnection(qintptr socketDescriptor)
{
	qDebug() << QString::fromLocal8Bit("新的连接")
				<< socketDescriptor << endl;

	//重新创建套接字来接受新的连接
	TcpSocket* tcpsocket = new TcpSocket();				//创建套接字
	tcpsocket->setSocketDescriptor(socketDescriptor);	//设置套接字描述符
	tcpsocket->run();			//设置套接字描述符,套接字有数据来执行的方法,套接字断开连接执行的方法

	//服务端套接字收到客户端发来数据的信号,server进行处理
	connect(tcpsocket, SIGNAL(signalGetDataFromClient(QByteArray&, int)),
				this, SLOT(SocketDataProcessing(QByteArray&, int)));
	 
	//客户端断开连接后,server进行处理
	connect(tcpsocket, SIGNAL(signalCilentDisconnect(int)),
				this, SLOT(SocketDisconnected(int)));

	//将socket添加到链表中
	m_tcpSocketConnectList.append(tcpsocket);
}

//数据处理
void TcpServer::SocketDataProcessing(QByteArray& SendData, int descriptor) {

	for (int i = 0; i < m_tcpSocketConnectList.count(); i++)
	{
		//从连接来的套接字链表循环读取信息
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		if (item->socketDescriptor() == descriptor) {
			qDebug() << QString::fromLocal8Bit("来自IP:%1 发来的数据:%2")
				.arg(item->peerAddress().toString())
				.arg(QString(SendData));
				    
			//发送客户端有链接过来信号
			emit signalTcpMsgComes(SendData);
		}
	}
}

//断开连接
void TcpServer::SocketDisconnected(int descriptor)
{
	//循环对客户端连接来套接字链表匹配要断开的描述符
	for (int i = 0; i < m_tcpSocketConnectList.count(); ++i) {
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		int itemDescriptor = item->socketDescriptor();

		if (itemDescriptor == descriptor || itemDescriptor == -1) {
			//断开
			m_tcpSocketConnectList.removeAt(i);//断开socket从链表中移除
			item->deleteLater();//回收资源
			qDebug() << QString::fromLocal8Bit("TcpSocket断开连接:") << descriptor << endl;
			return;
		}
	}
}