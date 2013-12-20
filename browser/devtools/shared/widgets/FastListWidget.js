const EventEmitter = require("devtools/shared/event-emitter");
const { Cu, Ci } = require("chrome");
const { ViewHelpers } = Cu.import("resource:///modules/devtools/ViewHelpers.jsm", {});

/**
 * A list menu widget that attempts to be very fast.
 *
 * Note: this widget should be used in tandem with the WidgetMethods in
 * ViewHelpers.jsm.
 *
 * @param nsIDOMNode aNode
 *        The element associated with the widget.
 */
const FastListWidget = module.exports = function FastListWidget(aNode) {
  this.document = aNode.ownerDocument;
  this.window = this.document.defaultView;
  this._parent = aNode;
  this._fragment = this.document.createDocumentFragment();

  // This is a prototype element that each item added to the list clones.
  this._templateElement = this.document.createElement("hbox");

  // Create an internal scrollbox container.
  this._list = this.document.createElement("scrollbox");
  this._list.className = "fast-list-widget-container theme-body";
  this._list.setAttribute("flex", "1");
  this._list.setAttribute("orient", "vertical");
  this._list.setAttribute("tabindex", "0");
  this._list.addEventListener("keypress", e => this.emit("keyPress", e), false);
  this._list.addEventListener("mousedown", e => this.emit("mousePress", e), false);
  this._parent.appendChild(this._list);

  this._orderedMenuElementsArray = [];
  this._itemsByElement = new Map();

  // This widget emits events that can be handled in a MenuContainer.
  EventEmitter.decorate(this);

  // Delegate some of the associated node's methods to satisfy the interface
  // required by MenuContainer instances.
  ViewHelpers.delegateWidgetEventMethods(this, aNode);
}

FastListWidget.prototype = {
  /**
   * Inserts an item in this container at the specified index, optionally
   * grouping by name.
   *
   * @param number aIndex
   *        The position in the container intended for this item.
   * @param nsIDOMNode aContents
   *        The node to be displayed in the container.
   * @param Object aAttachment [optional]
   *        Extra data for the user.
   * @return nsIDOMNode
   *         The element associated with the displayed item.
   */
  insertItemAt: function(aIndex, aContents, aAttachment={}) {
    let element = this._templateElement.cloneNode();
    element.appendChild(aContents);

    if (aIndex >= 0) {
      throw new Error("FastListWidget only supports appending items.");
    }

    this._fragment.appendChild(element);
    this._orderedMenuElementsArray.push(element);
    this._itemsByElement.set(element, this);

    return element;
  },

  /**
   * This is a non-standard widget implementation method. When appending items,
   * they are queued in a document fragment. This method appends the document
   * fragment to the dom.
   */
  flush: function() {
    this._list.appendChild(this._fragment);
  },

  /**
   * Removes all of the child nodes from this container.
   */
  removeAllItems: function() {
    let parent = this._parent;
    let list = this._list;

    while (list.hasChildNodes()) {
      list.firstChild.remove();
    }

    this._selectedItem = null;

    this._orderedMenuElementsArray.length = 0;
    this._itemsByElement.clear();
  },

  /**
   * Remove the given item.
   */
  removeChild: function(child) {
    throw new Error("Not yet implemented");
  },

  /**
   * Gets the currently selected child node in this container.
   * @return nsIDOMNode
   */
  get selectedItem() this._selectedItem,

  /**
   * Sets the currently selected child node in this container.
   * @param nsIDOMNode child
   */
  set selectedItem(child) {
    let menuArray = this._orderedMenuElementsArray;

    if (!child) {
      this._selectedItem = null;
    }
    for (let node of menuArray) {
      if (node == child) {
        node.classList.add("selected");
        node.parentNode.classList.add("selected");
        this._selectedItem = node;
      } else {
        node.classList.remove("selected");
        node.parentNode.classList.remove("selected");
      }
    }

    this.ensureElementIsVisible(this.selectedItem);
  },

  /**
   * Returns the child node in this container situated at the specified index.
   *
   * @param number index
   *        The position in the container intended for this item.
   * @return nsIDOMNode
   *         The element associated with the displayed item.
   */
  getItemAtIndex: function(index) {
    return this._orderedMenuElementsArray[index];
  },

  /**
   * Returns the value of the named attribute on this container.
   *
   * @param string name
   *        The name of the attribute.
   * @return string
   *         The current attribute value.
   */
  getAttribute: function(name) {
    return this._parent.getAttribute(name);
  },

  /**
   * Adds a new attribute or changes an existing attribute on this container.
   *
   * @param string name
   *        The name of the attribute.
   * @param string value
   *        The desired attribute value.
   */
  setAttribute: function(name, value) {
    this._parent.setAttribute(name, value);
  },

  /**
   * Removes an attribute on this container.
   *
   * @param string name
   *        The name of the attribute.
   */
  removeAttribute: function(name) {
    this._parent.removeAttribute(name);
  },

  /**
   * Ensures the specified element is visible.
   *
   * @param nsIDOMNode element
   *        The element to make visible.
   */
  ensureElementIsVisible: function(element) {
    if (!element) {
      return;
    }

    // Ensure the element is visible but not scrolled horizontally.
    let boxObject = this._list.boxObject.QueryInterface(Ci.nsIScrollBoxObject);
    boxObject.ensureElementIsVisible(element);
    boxObject.scrollBy(-element.clientWidth, 0);
  },

  window: null,
  document: null,
  _parent: null,
  _list: null,
  _selectedItem: null,
  _orderedMenuElementsArray: null,
  _itemsByElement: null
};
