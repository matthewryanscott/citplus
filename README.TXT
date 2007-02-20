Citadel+ Source Code Help
~~~~~~~~~~~~~~~~~~~~~~~~~

    When I was actively developing Citadel+, I knew which file held
    every function, what each function did, and what each function
    relied upon.  However, after coming back to it after a number of
    years away, I noticed that I was having a hard time getting
    around. Using tools like grep and some memory, I did okay, but it
    really struck me how much I didn't know the code any more when it
    took me over a day to properly implement a new CONFIG.CIT keyword:
    I had forgotten all of the places the need changing to implement
    it correctly.  I can only guess that others would have even more
    problems than me when it comes to maintaining this code.  As I'm
    distributing source freely now, it seems like I should at least
    make some effort to help newcomers to the code.  Over time, if I
    keep fiddling with the code, this might grow into some good
    documentation.

    VERSION CONTROL

        If you just want to dink around with the code, then of course
        you don't need to know what I'm doing, and I don't need to
        know what you're doing.  Similarly, if you want to take the
        code down your own path without ever wanting to make your new
        code work with my new code, we also don't need to know what
        each other are doing.

        However, if you want to have your changes be incorporated back
        into my code base, or have my changes incorporated into your
        code base, there are a few issues to be aware of.  Some are
        obvious ones that apply whenever you're working
        colloboratively: don't change the formatting of the code just
        for the sake of changing the formatting; don't make any major
        changes without first consulting with me (or anybody else who
        might be working on the code); etc.
        
        However, there are some rules to follow that are particular to
        this code base.  First, don't move things around or in the
        various *.MSG files or add new messages.  I will be doing
        this, and I've found that if more than one person is working
        in them, integrating the changes can be challenging.  This is
        because these files rely on line numbers, which are not in
        the files.  If you need to add a new string, just add it as a
        regular string literal in your code; I'll move it into a *.MSG
        file.

        There probably is a second (and more), but I cannot think any
        right now.  We'll work on this if it ever comes up.  If you do
        want to make changes and give them back to me, please let me
        know so I can avoid making big changes which will make it
        harder to incorporate your code back in.

    COMPILER INFORMATION

        I compile this with Borland C++ 5.01.  I think I might have a
        5.02 CD around somewhere, but I've misplaced it.  Previous
        releases of Citadel+ were compiled with earlier releases of
        Borland C++.  I have made some changes in this code to make
        it incompatible with Borland C++ 4.5, which was used for
        Citadel+/065. In particular, the scope of the variable i in
        this code has changed:

            for (int i = 1; i < 10; i++);

        In Borland C++ 4.5, i was still in scope after this line
        had completed.  By the time 5.0 was released, the C++
        standard had changed so that i is no longer in scope after
        the for() loop had completed.  I like this more, so I use
        that option.

        I have not compiled a DOS version for a while, so I don't know
        if it will compile and link cleanly.  It should at least be
        close:  I'm trying not to make any changes that will break
        it.

        I'm fairly sure that I'm using at least one Borland-specific
        run time library function in Citadel, so if you want to use
        another compiler, you might have some work ahead of you.

    GENERAL SOURCE LAYOUT AND NOTES

        Set your text editor to four-space tabs.  If your text editor
        does not have this option, get one that does.  I still have
        not found a text editor that I like as much as BRIEF.  I
        doubt that Borland still sells it, but if you run across it at
        a used software store, buy it. If it supported long file
        names, then I wouldn't use any other text editor ever.

        I tend to make it a rule that I turn on all of my compiler's
        warnings, and make the code compile warning-free.  However,
        you will notice that the compiler generates a whole mess of
        warnings.  I could get rid of them quickly, but I've chosen
        to leave them in to remind me what still needs work in the
        Windows version.  I only mention this here so you know that
        I'm not nearly so sloppy as to use code that generates so
        many warnings.

        The first few lines of most source modules give some idea of
        what's in them.  Actually, the names should give some idea. 
        I've been moving functions around over time, so the comments
        as to what each function does might be in another source
        module right now. A good grep utility can be a good friend of
        yours.

        For the most part, the source should be fairly normal and
        followable if you're familiar with C++.  There are some
        oddities, however.  I've moved much of the static data (such
        as strings) out of the program and into an external file
        (which is then actually copied to the end of the CTDL.EXE
        file).  I've also done a few odd things to let me compile both
        a DOS and a Win32 version from the same code base.

        HEY, WHERE ARE ALL THE STRINGS?

            First, the external data file.  I moved the static data
            into this file for two reasons:  to save disk space and to
            save RAM when running Citadel.  The expense is that some
            functions are a bit slower as Citadel needs to load
            information from this file.  The only times I've been able
            to notice this slowness is when using the Sysop menu (F6
            or .S) or the .Enter Configuration (.EC) commands on a
            slow machine, such as an 8088-based system.

            This method saves RAM in that only static data that is
            currently needed is loaded. It saves disk space in two
            ways. First, I run the data through some data compression
            algorithms when I create the file.  This (as of the 66024
            revision) reduces the space needed from 392,116 bytes to
            215,773:  a 176,343 byte (45%) savings. Second, I moved
            many bits of data that were in external files (such as
            *.BLB and *.HLP files) in other Citadels into this file,
            which saves room because of the wasted space at the end of
            a file when it doesn't fully fill a disk cluster.  A nice
            side-effect is that sysops don't need to keep these files
            up to date:  they are automatically updated when they get
            a new CTDL.EXE.  They still have the option of using the
            files if they want to override the default files, of
            course.

            The external data file is generated by the program
            CTDLDAT.EXE.  The source to this program is in the files
            CTDLDAT.CPP, COMP.H, COMP.CPP, COMPX.H, and COMPX.CPP. 
            The COMP?.* files are also used as Citadel source files. 
            This program reads the file CTDLDATA.DEF (which in turn
            loads several other files) and generates CTDL.DAT.

            This CTDL.DAT is then appended to CTDL.EXE with the
            ADDDAT.EXE program, which has ADDDAT.C as its single
            source file.  It simply copies CTDL.DAT onto the end of
            CTDL.EXE, then writes a signature to the end of this that
            lets Citadel know that it has been modified this way, and
            where the data starts.

            One of the problems introduced by this is that it can be
            hard to find where in Citadel a string is displayed.  I
            created two batch files, SM.BAT and SML.BAT, to help in
            this regard.  Whenever Citadel uses a string from
            CTDL.MSG, it is accessed through the getmsg() function. 
            Whenever Citadel uses a string from one of the other .MSG
            files, it uses a macro named getXmsg(), where X tells
            which .MSG file to read. By being sure to never put a line
            break in these calls, SM.BAT can find them.  SML.BAT does
            the same, but directs the output to a file name LIST, so I
            can load it into a text editor to have a list of each
            occurance.

        WHAT'S ALL THIS TERMWINDOWMEMBER STUFF?

    ACTUALLY COMPILING THE CODE: MAKEFILE AND .BAT FILES

    ADDING CONFIG.CIT KEYWORDS
    

    ADDING USER OPTIONS
        LOG1.H, DATAFILE.H, syncdata.CPP, syncdata.h, LOG3.CPP, LOGOUT.CPP,
        default user
---
i see code in logrdwr.cpp to set the new options correctly. however, my guess is that this code was not always there, as it's commented with "// ok, fine..."

 The reason that some of the flags seem backwards in the datafile is that unused space in the datafile is guaranteed to be set to 0. so when i make a new
userlog or whatever option, i make sure that the default that i want is 0. so, for example, instead of SeeOwnChats, I would have had HideOwnChats. So the 0
default would have meant "Don't Hide" instead of "Don't See."

 However, the code in logrdwr.cpp that's there now (the "// ok, fine..." code) fixes this by comparing the version of Citadel that last saved that database
record with the version of Citadel that added that database record. So it can force it to on.

 You'll see in that section code that sets defaults for other new options that couldn't, for whatever reason, just default to 0.

----

Also, speaking of Auto-Idle, your choice of datatype for the setting in datafile.h breaks the DOS/Windows datafile compatibility.

 you chose int. In Borland's 16-bit compiler (used for DOS versions), an int is 16 bits. In Borland's 32-bit compiler (used for Win32 versions), an int is 32
bits.

 You'll note that everything else in that file is explicitly defined as short (16 bits in both) or long (32 bits in both).

 I changed it to long. long is excessive for AutoIdle, but as there are boards already out there using the Windows version, I didn't want to break their
databases by switching it to a short. Such is life.

