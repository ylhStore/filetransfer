PROTO_FILES=*.proto

.PHONY:msg
msg:
	protoc --cpp_out=./ $(PROTO_FILES)

clean:
	rm -f *.pb.cc *pb.h
