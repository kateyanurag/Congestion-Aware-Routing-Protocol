
default:	
	@cd _arbiter/src && $(MAKE)
	@echo "Arbiter executable created"
	@echo "Run arbiter as ./arbiter"
	@cd ../..
	@cd _host/src && $(MAKE)
	@echo "Host executable created"
	@echo "Run host as ./cperf"

server:	
	@cd _arbiter/src && $(MAKE)
	@echo "Arbiter executable created"
	@echo "Run arbiter as ./arbiter"
host:
	@cd _host/src && $(MAKE)
	@echo "Host executable created"
	@echo "Run host as ./cperf"

#.PHONY: clean
clean:
	@echo "Cleaning Arbiter ...."
	@rm -f _arbiter/obj/*.o
	@rm -f _arbiter/build/arbiter
	@echo "Cleaning Host ...."
	@rm -f _host/obj/*.o
	@rm -f _host/build/cperf
	@echo "Removing Logs ...."	
	@rm -f logs/*
	@rm -f log/*
	@echo "Cleaning Complete."
	
