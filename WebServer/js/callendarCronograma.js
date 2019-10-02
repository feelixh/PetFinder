   function initAll(){
   $(function() {

    $('#calendar').fullCalendar({
		eventLimit: 6, // for all non-agenda views
		selectable: false,
  editable: true,
	  header: {
        left: 'prev,next, today',
        center: 'title',
        right: 'agendaWeek,month,listWeek'
      },
	   now: moment().format(),
	   nowIndicator: true,
	  views: {
      listWeek: { buttonText: 'Lista' }
    },
	  themeSystem: 'bootstrap4',
	   eventClick:  function(event, jsEvent, view) {
		   if (event.allDay==false&&event.aula==true){
			$('#modal-title').html(event.title+" - " + moment(event.start).format("HH:mm") +" às "+moment(event.end).format("HH:mm"));
			$('#identificador').val(event.id);
			$('#modalAula').modal('show');
		   }else if (event.allDay==true){
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
                 events: function(start, end, timezone, callback) {
    	 var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  	var events = [];
	  var registros = JSON.parse(this.responseText);
	  registros.forEach(item => {
		   events.push({
        title:item.nome,
        start: moment().day(item.dia).hours(item.horaini.substring(0, 2)).minute(item.horaini.substring(3, 5)).format(),
        end:moment().day(item.dia).hours(item.horafim.substring(0, 2)).minute(item.horafim.substring(3, 5)).format()
      });
		 
	  });
	   callback(events);
    }
  };
  xhttp.open("GET", "eventosCronograma.php", true);
  xhttp.send();
  },
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


  
  function update_event(eventElement){
	var evento = $("#calendar").fullCalendar('clientEvents', eventElement)[0];
	var start = moment(evento.start);
	var end = moment(evento.end);
	
	 if(evento.allDay){
		$('#startTime').val(null);
		$('#endTime').val(null);
		$('#title').val(evento.title);
		$('#description').val(evento.description);
		$('#startData').val(start.format("YYYY-MM-DD"));
		$('#endData').val(start.format("YYYY-MM-DD"));
		$('#modalTitleAddEvent').html("Adicionar evento no dia: "+start.format("DD/MM/YYYY"));
	 }else{
		$('#startTime').val(start.format("HH:mm:ss"));
	    $('#endTime').val(end.format("HH:mm:ss"));
		$('#title').val(evento.title);
		$('#description').val(evento.description);
		$('#startData').val(start.format("YYYY-MM-DD"));
		$('#endData').val(end.format("YYYY-MM-DD"));
		$('#modalTitleAddEvent').html("Adicionar evento do dia: "+start.format("DD/MM/YYYY")+" até "+end.format("DD/MM/YYYY"));
	 }		
			
	$('#calendarModalAddEvent').modal('show');
	
	
  }