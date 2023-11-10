#include "QTQQ_Server.h"
#include <qmessagebox.h>
#include <QSqlDatabase>
#include <qsqlrecord.h>
#include <QTimer>
#include <qfiledialog.h>
#include <QSqlQuery>
#include <qsqlquerymodel.h>

const int gTcpPort = 8888;
const int gUdpPort = 6666;

QTQQ_Server::QTQQ_Server(QWidget *parent)
    : QDialog(parent),
	m_pixPath("")
	
{
    ui.setupUi(this);
	//设置窗口标题
	setWindowTitle(QString::fromLocal8Bit("QTQQjxf服务端"));
	//连接数据库
	if (!connectMysql()) {
		QMessageBox::information(NULL,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("服务端连接数据库失败!"));
		close();
		return;

	}
	//设置图标
	QIcon icon(":/ps.png");
	setWindowIcon(icon);

	setStatusMap();
	setDepNameMap();
	setOnlineMap();

	//初始化组合框的数据
	initComboBoxData();

	//查找所有员工
	m_queryInfoModel.setQuery("SELECT * FROM tab_employees");
	
	//更新表格数据
	updateTabelData();


	//保存公司群ID号
	m_depID = getCompDepID();//开始部门选择公司群 即:列出所公司成员信息
	m_employeeID = 0;//初始化员工号为0
	m_compDepID = m_depID;
	
	//定时刷新数据
	m_timer = new QTimer(this);
	m_timer->setInterval(200);
	
	connect(m_timer, &QTimer::timeout, this, &QTQQ_Server::onRefresh);
	m_timer->start();

	//初始化tcp
	initTcpSocket();
	//初始化udp
	initUdpSocket();
}

QTQQ_Server::~QTQQ_Server()
{}


//初始化组合框的数据
void QTQQ_Server::initComboBoxData()
{
	QString itemTetx;//组合框项的文本

	//获取公司总的部门数
	QSqlQueryModel queryDepModel;
	queryDepModel.setQuery("SELECT * FROM tab_department");
	int depCount = queryDepModel.rowCount() - 1;//获取的记录去掉"公司群"不作为部门分支
	for (int i = 0; i < depCount; i++)
	{
		itemTetx = ui.employeeDepBox->itemText(i);//获取"员工部门"组合框项的文本
		QSqlQuery queryDepID(QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(itemTetx));
		queryDepID.exec();
		queryDepID.next();
			//设置员工所属部门组合框的数据为相应的部门号
		ui.employeeDepBox->setItemData(i, queryDepID.value(0).toInt());
		
	}
	//多一个"公司群"部门
	for (int i = 0; i < depCount + 1; i++) {
		itemTetx = ui.departmentBox->itemText(i);
		QSqlQuery queryDepID(QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(itemTetx));
		queryDepID.exec();
		queryDepID.next();
		//设置部门组合框的对应数据
		ui.departmentBox->setItemData(i, queryDepID.value(0).toInt());
		
		
	}
} 

void QTQQ_Server::initTcpSocket()
{
	m_tcpServer = new TcpServer(gTcpPort);//设置服务端端口号
	m_tcpServer->run();//套接字设置监听

	//收到客户端发来TCP的信息后进行UDP广播
	connect(m_tcpServer, &TcpServer::signalTcpMsgComes,
		this, &QTQQ_Server::onUDPbroadMsg);
}


bool QTQQ_Server::connectMysql()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setDatabaseName("qtqq");//数据库名称
	db.setHostName("localhost");//主机名
	db.setUserName("root");//用户名
	db.setPassword("atpx4869");//密码
	db.setPort(3306);//端口号
	if (db.open()) {
		return true;
	}
	else {
		return false;
	}
}
void QTQQ_Server::updateTabelData(int DepId, int employeeID)
{
	ui.tableWidget->clear();
	if (DepId && DepId != m_compDepID) {//查找部门(部门不包括公司群号)
		m_queryInfoModel.setQuery(
			QString("SELECT * FROM tab_employees WHERE tab_employees.departmentID = %1").arg(DepId));
	}
	else if (employeeID) {//精确查找员工号
		m_queryInfoModel.setQuery(
			QString("SELECT * FROM tab_employees WHERE employeeID = %1").arg(employeeID));
	}
	else {//查询所有
		m_queryInfoModel.setQuery(
			QString("SELECT * FROM tab_employees"));
	}

	int rows    = m_queryInfoModel.rowCount();//总行数
	int cloumns = m_queryInfoModel.columnCount();//总列数

	QModelIndex index;//模型索引
	//设置表格行数与列数
	ui.tableWidget->setRowCount(rows);
	ui.tableWidget->setColumnCount(cloumns);


	//设置表头
	QStringList headers;
	headers <<  QStringLiteral("部门") <<
				QStringLiteral("工号") <<
				QStringLiteral("员工姓名") <<
				QStringLiteral("员工签名") <<
				QStringLiteral("员工状态") <<
				QStringLiteral("员工头像") <<
				QStringLiteral("在线状态");
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置列等宽
	ui.tableWidget->setHorizontalHeaderLabels(headers);//设置水平表头

	
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cloumns; j++) {
			//获取第i行j列 模型索引
			index = m_queryInfoModel.index(i, j);
			//获取第i行j列 数据
			QString strData = m_queryInfoModel.data(index).toString();
			//获取i行 的记录
			QSqlRecord record = m_queryInfoModel.record(i);//当前行的记录
			//获取所在行的第j列的数据
			QString strRecordName = record.fieldName(j);//列
			if (strRecordName == QLatin1String("departmentID")) {
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_depNameMap.value(strData)));
				continue;
			}
			if (strRecordName == QLatin1String("status")) {
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_statusMap.value(strData)));
				continue;

			}if (strRecordName == QLatin1String("online")) {
				ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_onlineMap.value(strData)));
				continue;
			}
			//设置每一项
			ui.tableWidget->setItem(i, j, new QTableWidgetItem(strData));

		}
	}


}
int  QTQQ_Server::getCompDepID()
{
	QSqlQuery queryCompDepId(QString("select departmentId from tab_department where department_name ='%1'")
		.arg(QString::fromLocal8Bit("公司群")));
	queryCompDepId.exec();
	queryCompDepId.first();
	int compDepID = queryCompDepId.value(0).toInt();
	return compDepID;
}
void QTQQ_Server::setStatusMap()
{
	m_statusMap.insert(QStringLiteral("1"), QStringLiteral("有效"));
	m_statusMap.insert(QStringLiteral("0"), QStringLiteral("已注销"));
}
void QTQQ_Server::setDepNameMap()
{
	m_depNameMap.insert(QStringLiteral("2001"), QStringLiteral("人事部"));
	m_depNameMap.insert(QStringLiteral("2002"), QStringLiteral("研发部"));
	m_depNameMap.insert(QStringLiteral("2003"), QStringLiteral("市场部"));

}
void QTQQ_Server::setOnlineMap() 
{
	m_onlineMap.insert(QStringLiteral("1"), QStringLiteral("在线"));
	m_onlineMap.insert(QStringLiteral("2"), QStringLiteral("离线"));
	m_onlineMap.insert(QStringLiteral("3"), QStringLiteral("隐身"));
}
void QTQQ_Server::onRefresh()
{
	updateTabelData(m_depID, m_employeeID);
}
void QTQQ_Server::on_queryDepartmentBtn_clicked()//组合框查询 公司群/人事部/研发部/市场部
{
	ui.queryIDLineEdit->clear();
	m_employeeID = 0;
	m_depID = ui.departmentBox->currentData().toInt();
	updateTabelData(m_depID, m_employeeID);
}
void QTQQ_Server::on_queryIDBtn_clicked()		//员工号精确查找
{  
	ui.departmentBox->setCurrentIndex(0);
	m_depID = m_compDepID;
	//检测员工QQ号用户是否输入正确
	if (!ui.queryIDLineEdit->text().length()) {
		QMessageBox::information(this,
			QStringLiteral("提示"), 
			QStringLiteral("请输入员工QQ号"));
		ui.queryIDLineEdit->setFocus();
		return;
	}

	int employeeID = ui.queryIDLineEdit->text().toInt();
	QSqlQuery queryInfoModel(QString("SELECT* FROM tab_employees WHERE employeeID = %1 ").arg(employeeID));
	queryInfoModel.exec(); 

	if (!queryInfoModel.next()) {
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入正确的QQ号"));
		ui.queryIDLineEdit->clear();
		ui.queryIDLineEdit->setFocus();
		return;
	}
	else {
		m_employeeID = employeeID;
		ui.queryIDLineEdit->clear();
	}
}

void QTQQ_Server::on_LogoutBtn_clicked()//注销员工号
{
	//清理用QQ号查询编辑框输入的内容
	ui.queryIDLineEdit->clear();
	//选择组合框选中第一项
	ui.departmentBox->setCurrentIndex(0); 

	//注销员工QQ号是否输入正确
	if (!ui.logoutIDlineEdit->text().length()) {
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入员工QQ号"));

		ui.logoutIDlineEdit->setFocus();
		return;
	}
	int employeeID = ui.logoutIDlineEdit->text().toInt();
	QSqlQuery queryInfo(QString("SELECT employee_name FROM tab_employees where employeeID = %1  ").arg(employeeID));
	queryInfo.exec();
	if (!queryInfo.next()) {
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入正确的QQ号"));
		ui.logoutIDlineEdit->clear();
		ui.logoutIDlineEdit->setFocus();
		return;
	}
	else {
		
		QSqlQuery sqlUpdata(QString("UPDATE tab_employees SET status = 0 WHERE employeeID = %1").arg(employeeID));
		sqlUpdata.exec();
		QString strname = queryInfo.value(0).toString();
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("员工%1的企业QQ号:%2已被注销!")
			.arg(strname)
			.arg(employeeID));
		ui.logoutIDlineEdit->clear();
	}
}

void QTQQ_Server::on_selectPictureBtn_clicked()
{
	//获取选择的图像路径
	m_pixPath = QFileDialog::getOpenFileName(this,//继承于
		QString::fromLocal8Bit("选择头像"),//对话框的标题栏
		"./resource",//从当前路径下选择
		"*.png;;*jpg"//选择文件格式(用二个;隔开)
	);
	if (!m_pixPath.size()) {
		return;
	}
	QPixmap pixmap;
	pixmap.load(m_pixPath);
	qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
	qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();
	QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);
	ui.headLabel->setPixmap(pixmap.scaled(size));
}

void QTQQ_Server::on_addBtn_clicked()
{
	//检测员工姓名的输入
	QString strName = ui.nameLineEdit->text();
	if (!strName.size()) {
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("请输入员工姓名"));
		ui.nameLineEdit->setFocus();
		return;
	}
	//检测员工选择的头像
	if (!m_pixPath.size()) {
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("请选择员工头像"));
		return;

	}
	//数据库插入新的员工数据
	//获取员工QQ号
	QSqlQuery maxEmployeeID("SELECT max(employeeID) FROM tab_employees");
	maxEmployeeID.exec();
	maxEmployeeID.first();
	//新的QQ号
	int employeeID = maxEmployeeID.value(0).toInt() + 1;
	//新增员工部门QQ号
	int depID = ui.employeeDepBox->currentData().toInt();
	//新增员工签名
	QString employeeSignal = ui.signalLineEdit->text();
	//新增员工的在线状态
	QString strOnline = ui.onlineComboBox->currentText();
	QStringList onlinestrList = strOnline.split(".");
	int onlineDex = onlinestrList.at(0).toInt();//在线状态编号
	m_pixPath.replace("/", "\\\\");//   \\转义符\转成"\"
	//新的QQ号插入数据库中 
	QSqlQuery insertSql(QString("INSERT INTO tab_employees(departmentID,employeeID,employee_name,employee_sign,picture,online)\
			VALUES(%1, %2,'%3','%4','%5',%6)")
				.arg(depID)
				.arg(employeeID)
				.arg(strName)
				.arg(employeeSignal)
				.arg(m_pixPath)
				.arg(onlineDex));
	if (insertSql.exec()) {
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("新增员工%1(%2)成功!").arg(strName).arg(employeeID));
	}
	m_pixPath = "";
	ui.headLabel->setText(QString::fromLocal8Bit("员工寸照"));
	ui.signalLineEdit->clear();
	ui.nameLineEdit->clear();
	ui.employeeDepBox->setCurrentIndex(0);
	ui.onlineComboBox->setCurrentIndex(0);
}
void QTQQ_Server::initUdpSocket()
{
	m_udpsender = new QUdpSocket(this);
}
void QTQQ_Server::onUDPbroadMsg(QByteArray& btDate)
{
	for (qint16 port = gUdpPort; port < gUdpPort + 200; ++port) {//给客户端端口号在6666到6800进行广播
		//服务端通过端口号port进行广播数据
		m_udpsender->writeDatagram(btDate, btDate.size(), QHostAddress::Broadcast, port);
	}
}