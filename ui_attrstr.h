typedef struct uiDrawTextLayout uiDrawTextLayout;
typedef struct uiDrawTextLayoutParams uiDrawTextLayoutParams;
typedef struct uiDrawTextLayoutLineMetrics uiDrawTextLayoutLineMetrics;

_UI_ENUM(uiDrawTextAlign) {
	uiDrawTextAlignLeft,
	uiDrawTextAlignCenter,
	uiDrawTextAlignRight,
};

struct uiDrawTextLayoutParams {
	uiAttributedString *String;
	uiDrawFontDescriptor *DefaultFont;
	double Width;
	uiDrawTextAlign Align;
};

// Height will equal ParagraphSpacingBefore + LineHeightSpace + Ascent + Descent + Leading + LineSpacing + ParagraphSpacing.
// The above values are listed in vertical order, from top to bottom.
// Ascent + Descent + Leading will give you the typographic bounds
// of the text. BaselineY is the boundary between Ascent and Descent.
// X, Y, and BaselineY are all in the layout's coordinate system, so the
// start point of the baseline will be at (X, BaselineY). All values are
// nonnegative.
struct uiDrawTextLayoutLineMetrics {
	// This describes the overall bounding box of the line.
	double X;
	double Y;
	double Width;
	double Height;

	// This describes the typographic bounds of the line.
	double BaselineY;
	double Ascent;
	double Descent;
	double Leading;

	// This describes any additional whitespace.
	// TODO come up with better names for these.
	double ParagraphSpacingBefore;
	double LineHeightSpace;
	double LineSpacing;
	double ParagraphSpacing;

	// TODO trailing whitespace?
};

// TODO
// - allow creating a layout out of a substring
// - allow marking compositon strings
// - allow marking selections, even after creation
// - add the following functions:
// 	- uiDrawTextLayoutHeightForWidth() (returns the height that a layout would need to be to display the entire string at a given width)
// 	- uiDrawTextLayoutRangeForSize() (returns what substring would fit in a given size)
// 	- uiDrawTextLayoutNewWithHeight() (limits amount of string used by the height)
// - some function to fix up a range (for text editing)
_UI_EXTERN uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *params);
_UI_EXTERN void uiDrawFreeTextLayout(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y);
_UI_EXTERN void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height);
_UI_EXTERN int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end);
_UI_EXTERN void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m);
// TODO number of lines visible for clipping rect, range visible for clipping rect?

// TODO rewrite all this documentation

// uiDrawTextLayoutHitTest() returns the byte offset and line closest
// to the given point. The point is relative to the top-left of the layout.
// If the point is outside the layout itself, the closest point is chosen;
// this allows the function to be used for cursor positioning with the
// mouse. Do keep the returned line in mind if used in this way; the
// user might click on the end of a line, at which point the cursor
// might be at the trailing edge of the last grapheme on the line
// (subject to the operating system's APIs).
_UI_EXTERN void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *pos, int *line);

// uiDrawTextLayoutByteLocationInLine() returns the point offset
// into the given line that the given byte position stands. This is
// relative to the line's X position (as returned by
// uiDrawTextLayoutLineGetMetrics()), which in turn is relative to
// the top-left of the layout. This function can be used for cursor
// positioning: if start and end are the start and end of the line
// (as returned by uiDrawTextLayoutLineByteRange()), you will get
// the correct offset, even if pos is at the end of the line. If pos is not
// in the range [start, end], a negative value will be returned,
// indicating you need to move the cursor to another line.
// TODO make sure this works right for right-aligned and center-aligned lines and justified lines and RTL text
_UI_EXTERN double uiDrawTextLayoutByteLocationInLine(uiDrawTextLayout *tl, size_t pos, int line);

_UI_EXTERN void uiDrawCaret(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout, size_t pos, int *line);
// TODO allow blinking
// TODO allow secondary carets

typedef struct uiFontButton uiFontButton;
#define uiFontButton(this) ((uiFontButton *) (this))
// TODO have a function that sets an entire font descriptor to a range in a uiAttributedString at once, for SetFont?
_UI_EXTERN void uiFontButtonFont(uiFontButton *b, uiDrawFontDescriptor *desc);
// TOOD SetFont, mechanics
_UI_EXTERN void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data);
_UI_EXTERN uiFontButton *uiNewFontButton(void);
