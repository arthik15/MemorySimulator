all: memsim
	   
memsim: memsim.c	
         gcc memsim.c -o memsim

clean:
       rm memsim		 
	   