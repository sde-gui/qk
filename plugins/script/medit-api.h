#ifndef MEDIT_API_H
#define MEDIT_API_H

#ifdef __cplusplus

#include "momobject.h"

namespace mom {

class _Global;
class Object;
class Application;
class Window;
class Editor;
class EditWindow;
class Document;

class _Global
{
    MOM_DECLARE_OBJECT_CLASS(_Global)
public:
    Result getApplication(Application&) const;
    Result getEditor(Editor&) const;
    Result getActiveWindow(Window&) const;
    Result getActiveDocument(Document&) const;
};

class Object : public _ObjectImpl
{
    MOM_DECLARE_OBJECT_CLASS(Object)
public:
    Result getObjectName(String&) const;
};

class Application : public Object
{
    MOM_DECLARE_OBJECT_CLASS(Application)
public:
    Result getActiveWindow(Window&) const;
    Result setActiveWindow(const Window&);
    Result getWindows(List<Window>&) const;
};

class Window : public Object
{
    MOM_DECLARE_OBJECT_CLASS(Window)
public:
    Result isMaximized(bool&) const;
    Result setMaximized(bool);
    Result isMinimized(bool&) const;
    Result setMinimized(bool);
};

class Editor : public Object
{
    MOM_DECLARE_OBJECT_CLASS(Editor)
public:
    Result getActiveDocument(Document&) const;
    Result setActiveDocument(const Document&);
    Result getDocuments(List<Document>&) const;
    Result getActiveWindow(EditWindow&) const;
    Result setActiveWindow(const EditWindow&);
    Result getWindows(List<EditWindow>&) const;
};

class EditWindow : public Window
{
    MOM_DECLARE_OBJECT_CLASS(EditWindow)
public:
    Result getActiveDocument(Document&) const;
    Result setActiveDocument(const Document&);
    Result getDocuments(List<Document>&) const;
};

class Document : public Object
{
    MOM_DECLARE_OBJECT_CLASS(Document)
public:
    Result getWindow(EditWindow&) const;
};

} // namespace mom

#endif /* __cplusplus */

#endif /* MEDIT_API_H */
