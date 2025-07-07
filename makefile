NAME = apache
CXX = c++
CFLAGS = -Wall -Wextra -Werror -std=c++11 -pedantic

SRCS = utils/stringNumber.cpp server/Iconnect.cpp server/server.cpp request/request.cpp reponse/AResponse.cpp \
reponse/Response.cpp server/serverManager.cpp utils/trim.cpp reponse/cgiHandler.cpp \

HEADERS = includes/webserver.hpp includes/AResponse.hpp includes/Iconnect.hpp includes/Response.hpp \
includes/server.hpp includes/utils.hpp includes/webserver.hpp includes/Location.hpp \
includes/ServerManager.hpp includes/ConfigParser.hpp includes/Location.hpp includes/Connection.hpp \
includes/cgiHandler.hpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS) $(HEADERS)
	$(CXX) -g3 -fsanitize=address $(OBJS) -o $(NAME)
%.o: %.cpp 
	$(CXX) -c -g3 -fsanitize=address -o $@ $<

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
