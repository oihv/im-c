# im-c
An instant messaging toy project written in C.

## Usage Instruction
Follow the readme in both 'frontend/' and 'backend/' dir

## Testing

Run the comprehensive test suite:

```bash
./tests/run_tests.sh
```

Or run individual test categories:

```bash
cd tests/build
./test_message_types    # Unit tests for message handling
./test_textbox          # Unit tests for textbox components
./test_ui_components    # UI component logic tests
```

See `tests/README.md` for detailed testing documentation.

## TODO
- [x] figure out how to do automated testing, both backend and frontend (how to test ui, especially)
- [ ] from the automated test, generate test report

### frontend
#### textbox: nik
- [ ] manage focus (disables focus when clicking outside of the box)
- [ ] turn cursor back from IBEAM when not hovering a textbox
- [ ] integrate textbox to main page

#### connection (networking client) 
- [ ] simple authentication system (with websocket too? http) kenet bantu cek
- [ ] send message
- [ ] receive messages from server
- [ ] render messages based on username (whether its you or others thst sent the message)
- [ ] connecting to server loading, new page? handle connection retry and render in ui

#### UI
- [ ] interactivity (button color change when hovered and when pressed)
- [ ] font sizing (if possible, responsive based on window size) 

### backend
- [ ] implement system authentication system for handling username, i. e. handle how the server will name connecting clients, understand how the server differentiate between connected clients

##### Note
- [ ] serverside bakal ribetnya nanti pas lanjut roadmap, gmn cara manage multiple messages dr org ke org, simpen chat kevin ke ben, kevin ke steven, Steve ke ben, group chat

## ROADMAP
- [ ] personal chats
- [ ] group chats
- [ ] persistent storage with db
