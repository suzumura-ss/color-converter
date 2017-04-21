OUTDIR=Release
TARGET=$(OUTDIR)/color-converter
SRC=color-converter/ColorConverter.cpp color-converter/main.cpp
CXXFLAGS+=-Wall -std=c++11 `pkg-config --cflags opencv`
LDFLAGS +=`pkg-config --libs opencv`

.PHONY: all clean

all: $(TARGET)

clean:
	@rm -f $(OUTDIR)/*

$(TARGET): $(SRC)
	@mkdir -p $(OUTDIR)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^
