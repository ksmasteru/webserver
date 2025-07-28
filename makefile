NAME = apache
CC := c++
CFLAGS := -Wall -Wextra -Werror #-std=c++98

SRCS = utils/stringNumber.cpp server/Iconnect.cpp server/server.cpp request/request.cpp \
reponse/Response.cpp server/serverManager.cpp utils/trim.cpp reponse/cgiHandler.cpp \
reponse/Aresponse.cpp config/Location.cpp config/ConfigParser.cpp

HEADERS := includes/webserver.hpp includes/AResponse.hpp includes/Iconnect.hpp includes/Response.hpp \
includes/server.hpp includes/utils.hpp includes/webserver.hpp includes/Location.hpp \
includes/serverManager.hpp includes/ConfigParser.hpp includes/Location.hpp includes/Connection.hpp \
includes/cgiHandler.hpp
OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
