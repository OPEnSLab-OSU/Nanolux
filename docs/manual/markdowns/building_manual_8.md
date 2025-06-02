## Building the Nanolux Developer's Handbook ##

The file manual_builder.py is a neat script used to build the Nanolux Developer's Handbook from the markdown files present in the markdowns folder.

### Dependencies ###

Use the following commands to install all dependencies:

> pip install markdown

> pip install pdfkit

> pip install pypdf

You'll also need the file wkhtmltoppdf.exe from python-pdfkit. Get it from the 7z Archive Windows download at this link: https://wkhtmltopdf.org/downloads.html

Add this file to this folder. It is already in the .gitignore so it isn't committed to the repository.

This utility is only validated to run on Windows as of 5/22/2024.

### Adding New Sections ###

To add a new section to the manual, move the markdown file to the markdowns folder. Then, add the file name (omit the .md extension) to the "input\_filenames" list in manual\_builder.py:

	input_filenames = [
	    'title_0',
	    'toc_1',
	    'terminology_2',
	    'structs_3',
	    'project_struct_4',
	    'main_loop_5',
	    'add_patterns_6',
		'audio_analysis_7',
		'building_manual_8',
		'changelog_9'
	]

If inserting a section requires renaming others, change the numbering in both the python file and on all markdown files. Make sure to update the front cover (release date, version) and the changelog section.