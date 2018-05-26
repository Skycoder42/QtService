%modules = (
	"QtService" => "$basedir/src/service",
);

# Force generation of camel case headers for classes inside QtDataSync namespaces
$publicclassregexp = "QtService::(?!__helpertypes).+";
