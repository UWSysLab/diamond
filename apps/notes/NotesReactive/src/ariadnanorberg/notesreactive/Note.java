package ariadnanorberg.notesreactive;

public class Note {

    private String title;
    private String content;

    Note(String noteTitle, String noteContent) {
        title = noteTitle;
        content = noteContent;

    }

    public String getTitle() {
        return title;
    }
    public void setTitle(String title) {
        this.title = title;
    }
    public String getContent() {
        return content;
    }
    public void setContent(String content) {
        this.content = content;
    }
    @Override
    public String toString() {
        return this.getTitle();
    }
    
}