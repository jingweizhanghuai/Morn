// g++ test_map3.cpp -I C:\ProgramFiles\Qt_mingw\include\ -o test_map3.exe -L C:\ProgramFiles\Qt_ming w\lib -lQt5Core -lqtpcre2 -lwinmm -lws2_32 -lole32 -lversion -luuid
#include <QtCore/qmap.h>
#include <QtCore/qdebug.h>
 
int main(int argc, char *argv[])
{
	// QCoreApplication a(argc, argv);
 
	QMap<QString,int> map;
	map.insert("eins o",1);
	map.insert("sieben",7);
	map["a test"]=23;
	//Normally, a QMap allows only one value per key. 
	//If you call insert() with a key that already exists in the QMap, 
	//the previous value will be erased
	map["a test"]=13;
 
	int val=map.value("a test");
	printf("val=%d\n",val);
 
	int val2=map.value("error");
	printf("val2=%d\n",val2);
 
	int val3=map.value("error2",20);
	printf("val3=%d\n",val3);
 
	int val4=map.value("a test",40);
	printf("val4=%d\n",val4);
	
	// qDebug()<<val4;
	// using a java-style iterator
	QMapIterator<QString,int> iter(map);
	while(iter.hasNext())
	{
		iter.next();  ///放在前面
		qDebug()<<iter.key()<<" "<<iter.value();
	}
	// using an Stl-style iterator 
	QMap<QString,int>::ConstIterator i=map.constBegin();
	while(i!=map.constEnd())
	{
		qDebug()<<i.key()<<": "<<i.value();
		++i;
	}
 
	QMap<QString,int>::Iterator it;
	it=map.find("sieben");
	if(it!=map.end())
		it.value()=8;
 
	for(QMap<QString,int>::ConstIterator ite=map.constBegin(); ite!=map.constEnd(); ++ite)
		qDebug()<<ite.key()<<": "<<ite.value();
 
	return 0;//a.exec();
}