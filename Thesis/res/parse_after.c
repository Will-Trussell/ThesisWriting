GLOBAL bool Parse_Request( CONN_ID Idx, char *Request ) {
  WUFFS
    pub struct parser?(
      prefix: slice base.u8, cmd : slice base.u8,
      args : array[15] slice base.u8, argc : base.u32[..=15]
    )
    pub func parser.parse?(src: base.io_reader) {
      var c : base.u8 = args.src.read_u8?() //Code removing whitespace omitted
      var i : base.u32[..15] = 0
      var extra : base.u32
      if c == ':' {
        extra = 0
        while true {
          c = args.src.read_u8?()
          if c == ' ' {
            args.src.limited_copy_u32_to_slice!(up_to: extra, s: this.prefix)
            break
          }
          else { extra += 1 }
        }endwhile
        c = args.src.read_u8?() //Prep for reading command
      } else { //Handle command
        extra = 1
        while true {
          c = args.src.read_u8?()
          if c == ' ' {
            args.src.limited_copy_u32_to_slice!(up_to: extra, s: this.cmd)
            break
          } else { extra += 1 }
        }endwhile
      }
      c = args.src.read_u8?()
      if c == 0{ return } //No more args, we are finished
      if c == ' '{ /* Code moving past whitespace omitted */ }
      else { //At least one arg
        extra = 1
        while.loop1 true {
          while.loop2 true {
            c = args.src.read_u8?()
            if c == 0 { break.loop1 } // c is null, we are finished parsing
            if c == ' ' {
              args.src.limited_copy_u32_to_slice!(up_to: extra, s: this.args[i])
              extra = 0
              i += 1
              if i >= 14{ break.loop1 }
              else{ break.loop2 }
            } else { extra += 1 }
          }endwhile.loop2
        }endwhile.loop1
        this.argc = i+1
      }
    } WUFFS_END
} // Parse_Request
