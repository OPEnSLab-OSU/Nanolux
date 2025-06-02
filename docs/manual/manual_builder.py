from markdown import markdown
import pdfkit
from pypdf import PdfWriter

# Sources:
# https://stackoverflow.com/questions/3444645/merge-pdf-files
# https://www.pyphilly.org/generating-pdf-markdown-or-html/

input_filenames = [
    'title_0',
    'toc_1',
    'terminology_2',
    'structs_3',
    'project_struct_4',
    'main_loop_5',
    'add_patterns_7',
    'building_manual_8',
    'changelog_9'
]

css = 'style.css'

def make_pdfs():
    for file in input_filenames:
        print(file)
        with open('markdowns/' + file + '.md', 'r') as f:
            html_text = markdown(f.read(), output_format='html4')
        pdfkit.from_string(html_text, 'pdfs/' + file + '.pdf', css=css)

def merge_pdfs():
    merger = PdfWriter()
    for file in input_filenames:
        merger.append('pdfs/' + file + '.pdf')
    merger.write("Nanolux Developer's Handbook.pdf")
    merger.close()

make_pdfs()
merge_pdfs()