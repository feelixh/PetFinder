<?php
	// Variáveis de conexão
	$host = '187.109.226.100';
	$user = 'petfinder';
	$pass = 'petfinder2019';
	$base = 'petfinder';

	// Evita mensagens de "aviso"
	error_reporting(E_ALL & ~ E_NOTICE & ~ E_DEPRECATED);
	
	// Conecta no servidor MySQL
	if(!(mysql_connect($host,$user,$pass))) {
		// Se não conseguir conectar, exibe mensagem de erro
		echo "Erro! Host, Usuário ou Senha do MySQL incorreta.";
		// Interrompe a execução da aplicação
		exit;
	}
	
	// Conecta no banco de dados $base
	if(!(mysql_select_db($base))) {
		// Se não conseguir conectar, exibe mensagem de erro
		echo "Erro! Banco de dados não acessível.";
		// Interrompe a execução da aplicação
		exit;
	}
	
	// Ajusta os caracteres especiais
	mysql_query("SET NAMES 'utf8'");
mysql_query('SET character_set_connection=utf8');
mysql_query('SET character_set_client=utf8');
mysql_query('SET character_set_results=utf8');
	
?>