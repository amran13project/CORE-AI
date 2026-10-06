CORE-AI GUI final API fix

Replace only gui\app.js in the existing project.
The native backend expects POST /chat with JSON {"prompt":"...","mode":"chat"} or mode "think".
Health uses GET /status.
No C++ source is changed.
