ASCIIDOCTOR_PDF := asciidoctor-pdf
DOC := Course
RESULT_PDF := $(DOC).pdf

.PHONY: all pdf docker clean
all: pdf

pdf:
	$(ASCIIDOCTOR_PDF) $(DOC).adoc

docker:
	docker run --rm -v "$$(pwd)":/documents/ \
		asciidoctor/docker-asciidoctor asciidoctor-pdf $(DOC).adoc

clean:
	$(RM) $(RESULT_PDF)
