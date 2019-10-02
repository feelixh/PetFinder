<?php

// Verificar se o formulário foi submetido
if($_POST) {
	// Carrega arquivo
    include("conexao.php");
    
    print_r($_POST);
    /*
	// Seleciona do banco de dados
	$sql = "SELECT *
			FROM v_usuarios
			WHERE login = '$email'
				AND senha = '$senha';";
	// Executa a SQL
	$query = mysql_query($sql);

	
	// Se houver registro com as credenciais informadas
	if(mysql_num_rows($query)) {
		
		
	} else {
		print("err");
    }
    */
	// Formulário não submetido
} else {
	// Redireciona para a página
	header("Location: 403.php")
}
?>