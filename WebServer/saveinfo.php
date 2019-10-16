<?php

// Verificar se o formulário foi submetido
if(isset($_POST)) {
	// Carrega arquivo
   include("conexao.php");
    
	//print_r($_POST);
	$sql = "insert into node (id_node, id_server, latitude, longitude, date_node, time_node, date_server, time_server)
	values ";
	foreach ($_POST as $dados) { 
		//print $dados;
		list ($node_id, $server_id, $lat, $long, $date_node, $time_node) = split (';', $dados);
		$sql = $sql."('$node_id','$server_id','$lat','$long','$date_node','$time_node',CURDATE(),CURRENT_TIME()), ";
	 }
	$sql =(substr($sql,0,strripos($sql,",")));
	

	if(mysql_query($sql)) {
		print("ok");
		
	} else {
		print("err");
    }
    
	// Formulário não submetido
} else if( $_GET){
print_r($_GET);
}else {
	// Redireciona para a página
	//header("Location: 403.php");
	print("err");
}
?>