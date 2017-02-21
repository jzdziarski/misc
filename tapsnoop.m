/* tapsnoop.m
 * Jonathan Zdziarski
 * gcc -o tapsnoop tapsnoop.m -framework CoreGraphics -framework Foundation
 *
 * A convenient way to list keyboard and mouse taps, to see what processes
 * are tapping keypresses and mouse clicks; this of course won't catch
 * kernel-based kits, but a vast majority of all malware employs the use
 * of userland event taps, as kernel techniques are not typically very
 * portable, and less stable.
 */

#include <CoreGraphics/CoreGraphics.h>
#include <Foundation/Foundation.h>
#include <inttypes.h>
#include <libproc.h>

int main() {
    uint32_t maxNumberOfTaps = 0xff;
    CGEventTapInformation tapList[maxNumberOfTaps];
    uint32_t eventTapCount;
    uint32_t tapNumber;

    CGGetEventTapList(maxNumberOfTaps, tapList, &eventTapCount);
    printf("tap count: %d\n", eventTapCount);

    for(tapNumber = 0; tapNumber < eventTapCount; ++tapNumber) 
    {
        CGEventMask eventsOfInterest = tapList[tapNumber].eventsOfInterest;
        pid_t pid = tapList[tapNumber].tappingProcess;
        char pidpath[PATH_MAX];

        proc_pidpath(pid, pidpath, PATH_MAX);
        printf("pid %d (%s) listening on %d enabled %d\n", tapList[tapNumber].tappingProcess, pidpath, tapList[tapNumber].processBeingTapped, tapList[tapNumber].enabled);
	printf("\tevents[%lu]: ", eventsOfInterest);
        if (eventsOfInterest & CGEventMaskBit(kCGEventKeyDown)
         || eventsOfInterest & CGEventMaskBit(kCGEventKeyUp))
        {
            printf("keyboard ");
        }

        if (eventsOfInterest & CGEventMaskBit(kCGEventLeftMouseDown)
         || eventsOfInterest & CGEventMaskBit(kCGEventLeftMouseUp))
        {
            printf("mouse ");
        }
        printf("\n");
    }

    return 0;
}
