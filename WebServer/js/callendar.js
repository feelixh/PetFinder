   function initAll(){
   $(function() {

    $('#calendar').fullCalendar({
		eventLimit: false, // for all non-agenda views
		selectable: true,
  editable: true,
	  header: {
        left: 'prevYear,prev,next,nextYear today',
        center: 'title',
        right: 'agendaDay,agendaWeek,month'
      },
	  allDayDefault: false,
	  themeSystem: 'bootstrap4',
	  eventRender: function(event, element){
          element.popover({
              animation:true,
              delay: 300,
			   placement: 'auto',
			    html: true,
              content: '<strong>Clique para obter detalhes</strong>',
              trigger: 'hover'
          });
        },
	   eventClick:  function(event, jsEvent, view) {
		   console.log(event);
		   if (event.allDay===false && event.aula===true){
			$('#modal-title').html(event.title+" - " + moment(event.start).format("HH:mm") +" às "+moment(event.end).format("HH:mm"));
			$('#identificador').val(event.id);
			$('#modalAula').modal('show');
		   }else if (event.allDay===1){
			$('#modalTitle').html(event.title);
            $('#paragraph').html(event.description);
			$('#actions').html('<br><a id="'+event.id+'" onClick="update_event(id)"><i class="fas fa-edit">&nbsp</i></a>');
			$('#actions').append('<a id="'+event.id+'" onClick="delete_event(id)"><i class="fas fa-trash-alt">&nbsp</i></a>');
			$('#calendarModal').modal('show');
		   }else{
			$('#modalTitle').html(event.title+" - " + moment(event.start).format("HH:mm") +" às "+moment(event.end).format("HH:mm"));
            $('#paragraph').html(event.description);
			$('#actions').html('<a id="'+event.id+'" onClick="update_event(id)"><i class="fas fa-edit">&nbsp</i></a>');
			$('#actions').append('<a id="'+event.id+'" onClick="delete_event(id)"><i class="fas fa-trash-alt">&nbsp</i></a>');
			$('#calendarModal').modal('show');
		   }
		   $coment = event.comentario;
		   if(event.remarcar == true && event.aula == true){
			   $('#aulaDetalhe').html("<h4>Solicitação aguardando aprovação do moderador</h4>");
		   }else if(event.aula == true && $coment!=null){
			   $('#aulaDetalhe').html("<h4>"+$coment+"</h4>");
		   }else{
			   $('#aulaDetalhe').html("<h4>Nada consta</h4>");
		   }
		   
        },
		select: function(startDate, endDate) {
			var data1 = moment(startDate);
			var data2 = moment(endDate);
			$('#id').val(-1);
			if(data2.diff(data1, 'days')>1){
				data2 = data2.subtract("1", "days");
				$('#description').val("");
				$('#title').val("");
				$('#startData').val(startDate.format("YYYY-MM-DD"));
				$('#endData').val(data2.format("YYYY-MM-DD"));
				$('#modalTitleAddEvent').html("Adicionar evento do dia: "+startDate.format("DD/MM/YYYY")+" até "+data2.format("DD/MM/YYYY"));
			}else{
				$('#description').val("");
				$('#title').val("");
				$('#startData').val(startDate.format("YYYY-MM-DD"));
				$('#endData').val(startDate.format("YYYY-MM-DD"));
				$('#modalTitleAddEvent').html("Adicionar evento no dia: "+startDate.format("DD/MM/YYYY"));
			}
		  
	  $('#startTime').val(null);
	   $('#endTime').val(null);
	$('#calendarModalAddEvent').modal('show');
        },
                events: 'eventos.php', 
	/*
	 dayClick: function(date, jsEvent, view) {
	 $('#startData').val(date.format("YYYY-MM-DD"));
	  $('#startTime').val(null);
	   $('#endTime').val(null);
	 $('#endData').val(date.format("YYYY-MM-DD"));
	$('#modalTitleAddEvent').html("Adicionar evento no dia: "+date.format("DD/MM/YYYY"));
	$('#calendarModalAddEvent').modal('show');
	
  },*/
   

    });

  });
  
 
	}
  
  function delete_event(eventElement){
	 if (confirm('Tem certeza que quer deletar?')) {
	   var dados = {id:eventElement};
                $.ajax({
                    type: "POST",
                    url: "deletar_evento.php",
                    data: dados,
                    success: function(data){   
                        if(data == "1"){
                            alert("Deletado com sucesso! ");
                           $("#calendar").fullCalendar('refetchEvents');
						   $('#calendarModal').modal('hide');
                        }else{
                            alert(data);
                        }
                    }
                }); 
    };
  }
  
  function update_event(eventElement){
	var evento = $("#calendar").fullCalendar('clientEvents', eventElement)[0];
	var start = moment(evento.start);
	var end = moment(evento.end);
	var identif = evento.id;
	
	 if(evento.allDay){
		$('#startTime').val(null);
		$('#endTime').val(null);
		$('#title').val(evento.title);
		$('#description').val(evento.description);
		$('#startData').val(start.format("YYYY-MM-DD"));
		$('#endData').val(start.format("YYYY-MM-DD"));
		$('#id').val(identif);
		$('#modalTitleAddEvent').html("Alterar evento");
	 }else{
		$('#startTime').val(start.format("HH:mm:ss"));
	    $('#endTime').val(end.format("HH:mm:ss"));
		$('#title').val(evento.title);
		$('#description').val(evento.description);
		$('#startData').val(start.format("YYYY-MM-DD"));
		$('#endData').val(end.format("YYYY-MM-DD"));
		$('#id').val(identif);
		$('#modalTitleAddEvent').html("Alterar evento");
	 }		
			
	$('#calendarModalAddEvent').modal('show');
	
	
  }

       $(document).ready(function() {	
           	
            //CADASTRA NOVO EVENTO
            $('#novo_evento').submit(function(){
                var dados = jQuery(this).serialize();
                $.ajax({
                    type: "POST",
                    url: "cadastrar_evento.php",
                    data: dados,
                    success: function(data)
                    {   
                        if(data == "1"){
                            alert("Adicionado com sucesso! ");
                            $("#calendar").fullCalendar('refetchEvents');
							$('#calendarModalAddEvent').modal('hide');
							$('#calendarModal').modal('hide');
                        }else if(data == "2"){
							alert("Alterado com sucesso! ");
                            $("#calendar").fullCalendar('refetchEvents');
							$('#calendarModalAddEvent').modal('hide');
							$('#calendarModal').modal('hide');
						}else{
                            alert(data);
                        }
                    }
                });                
                return false;
            });
			$('#remarcarAula').submit(function(){
                var dados = jQuery(this).serialize();
                $.ajax({
                    type: "POST",
                    url: "remarcar_aula.php",
                    data: dados,
                    success: function(data)
                    {   
                        if(data == "1"){
                            alert("Solicitação efetuada com sucesso! ");
                            $("#calendar").fullCalendar('refetchEvents');
                        }else{
                            alert(data);
                        }
                    }
                });                
                return false;
            });
			
	   }); 
              