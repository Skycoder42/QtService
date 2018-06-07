#include "%{SvcHdrName}"

int main(int argc, char *argv[])
{
	%{SvcCn} svc{argc, argv};
	return svc.exec();
}
