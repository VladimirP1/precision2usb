/*
 * descriptors.h
 *
 *  Created on: Jul 5, 2019
 *      Author: vladimir
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

static const uint8_t hid_report_descriptor[] = {
		0x05, 0x01,                     /*  Usage Page (Desktop),                   */
		0x09, 0x02,                     /*  Usage (Mouse),                          */
		0xA1, 0x01,                     /*  Collection (Application),               */
		0x85, 0x01,                     /*      Report ID (1),                      */
		0x09, 0x01,                     /*      Usage (Pointer),                    */
		0xA1, 0x00,                     /*      Collection (Physical),              */
		0x05, 0x09,                     /*          Usage Page (Button),            */
		0x19, 0x01,                     /*          Usage Minimum (01h),            */
		0x29, 0x02,                     /*          Usage Maximum (02h),            */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x01,                     /*          Logical Maximum (1),            */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x95, 0x06,                     /*          Report Count (6),               */
		0x81, 0x03,                     /*          Input (Constant, Variable),     */
		0x05, 0x01,                     /*          Usage Page (Desktop),           */
		0x09, 0x30,                     /*          Usage (X),                      */
		0x09, 0x31,                     /*          Usage (Y),                      */
		0x15, 0x81,                     /*          Logical Minimum (-127),         */
		0x25, 0x7F,                     /*          Logical Maximum (127),          */
		0x75, 0x08,                     /*          Report Size (8),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x81, 0x06,                     /*          Input (Variable, Relative),     */
		0x75, 0x08,                     /*          Report Size (8),                */
		0x95, 0x05,                     /*          Report Count (5),               */
		0x81, 0x03,                     /*          Input (Constant, Variable),     */
		0xC0,                           /*      End Collection,                     */
		0x06, 0x00, 0xFF,               /*      Usage Page (FF00h),                 */
		0x09, 0x01,                     /*      Usage (01h),                        */
		0x85, 0x0E,                     /*      Report ID (14),                     */
		0x09, 0xC5,                     /*      Usage (C5h),                        */
		0x15, 0x00,                     /*      Logical Minimum (0),                */
		0x26, 0xFF, 0x00,               /*      Logical Maximum (255),              */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0x95, 0x04,                     /*      Report Count (4),                   */
		0xB1, 0x02,                     /*      Feature (Variable),                 */
		0xC0,                           /*  End Collection,                         */
		0x06, 0x00, 0xFF,               /*  Usage Page (FF00h),                     */
		0x09, 0x01,                     /*  Usage (01h),                            */
		0xA1, 0x01,                     /*  Collection (Application),               */
		0x85, 0x5C,                     /*      Report ID (92),                     */
		0x09, 0x01,                     /*      Usage (01h),                        */
		0x95, 0x0B,                     /*      Report Count (11),                  */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0x81, 0x06,                     /*      Input (Variable, Relative),         */
		0x85, 0x0D,                     /*      Report ID (13),                     */
		0x09, 0xC5,                     /*      Usage (C5h),                        */
		0x15, 0x00,                     /*      Logical Minimum (0),                */
		0x26, 0xFF, 0x00,               /*      Logical Maximum (255),              */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0x95, 0x04,                     /*      Report Count (4),                   */
		0xB1, 0x02,                     /*      Feature (Variable),                 */
		0x85, 0x0C,                     /*      Report ID (12),                     */
		0x09, 0xC6,                     /*      Usage (C6h),                        */
		0x96, 0x20, 0x03,               /*      Report Count (800),                 */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0xB1, 0x02,                     /*      Feature (Variable),                 */
		0x85, 0x0B,                     /*      Report ID (11),                     */
		0x09, 0xC7,                     /*      Usage (C7h),                        */
		0x95, 0x42,                     /*      Report Count (66),                  */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0xB1, 0x02,                     /*      Feature (Variable),                 */
		0xC0,                           /*  End Collection,                         */
		0x05, 0x0D,                     /*  Usage Page (Digitizer),                 */
		0x09, 0x05,                     /*  Usage (Touchpad),                       */
		0xA1, 0x01,                     /*  Collection (Application),               */
		0x85, 0x04,                     /*      Report ID (4),                      */
		0x09, 0x22,                     /*      Usage (Finger),                     */
		0xA1, 0x02,                     /*      Collection (Logical),               */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x01,                     /*          Logical Maximum (1),            */
		0x09, 0x47,                     /*          Usage (Touch Valid),            */
		0x09, 0x42,                     /*          Usage (Tip Switch),             */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x81, 0x03,                     /*          Input (Constant, Variable),     */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x75, 0x04,                     /*          Report Size (4),                */
		0x25, 0x0F,                     /*          Logical Maximum (15),           */
		0x09, 0x51,                     /*          Usage (Contact Identifier),     */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x01,                     /*          Usage Page (Desktop),           */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x26, 0xE2, 0x0C,               /*          Logical Maximum (3298),         */
		0x75, 0x10,                     /*          Report Size (16),               */
		0x55, 0x0E,                     /*          Unit Exponent (14),             */
		0x65, 0x13,                     /*          Unit (Inch),                    */
		0x09, 0x30,                     /*          Usage (X),                      */
		0x35, 0x00,                     /*          Physical Minimum (0),           */
		0x46, 0xA3, 0x01,               /*          Physical Maximum (419),         */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x46, 0xF6, 0x00,               /*          Physical Maximum (246),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x09, 0x31,                     /*          Usage (Y),                      */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x0D,                     /*          Usage Page (Digitizer),         */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x64,                     /*          Logical Maximum (100),          */
		0x95, 0x03,                     /*          Report Count (3),               */
		0xC0,                           /*      End Collection,                     */
		0x09, 0x22,                     /*      Usage (Finger),                     */
		0xA1, 0x02,                     /*      Collection (Logical),               */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x01,                     /*          Logical Maximum (1),            */
		0x09, 0x47,                     /*          Usage (Touch Valid),            */
		0x09, 0x42,                     /*          Usage (Tip Switch),             */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x81, 0x03,                     /*          Input (Constant, Variable),     */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x75, 0x04,                     /*          Report Size (4),                */
		0x25, 0x0F,                     /*          Logical Maximum (15),           */
		0x09, 0x51,                     /*          Usage (Contact Identifier),     */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x01,                     /*          Usage Page (Desktop),           */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x26, 0xE2, 0x0C,               /*          Logical Maximum (3298),         */
		0x75, 0x10,                     /*          Report Size (16),               */
		0x55, 0x0E,                     /*          Unit Exponent (14),             */
		0x65, 0x13,                     /*          Unit (Inch),                    */
		0x09, 0x30,                     /*          Usage (X),                      */
		0x35, 0x00,                     /*          Physical Minimum (0),           */
		0x46, 0xA3, 0x01,               /*          Physical Maximum (419),         */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x46, 0xF6, 0x00,               /*          Physical Maximum (246),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x09, 0x31,                     /*          Usage (Y),                      */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x0D,                     /*          Usage Page (Digitizer),         */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x64,                     /*          Logical Maximum (100),          */
		0x95, 0x03,                     /*          Report Count (3),               */
		0xC0,                           /*      End Collection,                     */
		0x09, 0x22,                     /*      Usage (Finger),                     */
		0xA1, 0x02,                     /*      Collection (Logical),               */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x01,                     /*          Logical Maximum (1),            */
		0x09, 0x47,                     /*          Usage (Touch Valid),            */
		0x09, 0x42,                     /*          Usage (Tip Switch),             */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x81, 0x03,                     /*          Input (Constant, Variable),     */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x75, 0x04,                     /*          Report Size (4),                */
		0x25, 0x0F,                     /*          Logical Maximum (15),           */
		0x09, 0x51,                     /*          Usage (Contact Identifier),     */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x01,                     /*          Usage Page (Desktop),           */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x26, 0xE2, 0x0C,               /*          Logical Maximum (3298),         */
		0x75, 0x10,                     /*          Report Size (16),               */
		0x55, 0x0E,                     /*          Unit Exponent (14),             */
		0x65, 0x13,                     /*          Unit (Inch),                    */
		0x09, 0x30,                     /*          Usage (X),                      */
		0x35, 0x00,                     /*          Physical Minimum (0),           */
		0x46, 0xA3, 0x01,               /*          Physical Maximum (419),         */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x46, 0xF6, 0x00,               /*          Physical Maximum (246),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x09, 0x31,                     /*          Usage (Y),                      */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x0D,                     /*          Usage Page (Digitizer),         */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x64,                     /*          Logical Maximum (100),          */
		0x95, 0x03,                     /*          Report Count (3),               */
		0xC0,                           /*      End Collection,                     */
		0x09, 0x22,                     /*      Usage (Finger),                     */
		0xA1, 0x02,                     /*      Collection (Logical),               */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x01,                     /*          Logical Maximum (1),            */
		0x09, 0x47,                     /*          Usage (Touch Valid),            */
		0x09, 0x42,                     /*          Usage (Tip Switch),             */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x81, 0x03,                     /*          Input (Constant, Variable),     */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x75, 0x04,                     /*          Report Size (4),                */
		0x25, 0x0F,                     /*          Logical Maximum (15),           */
		0x09, 0x51,                     /*          Usage (Contact Identifier),     */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x01,                     /*          Usage Page (Desktop),           */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x26, 0xE2, 0x0C,               /*          Logical Maximum (3298),         */
		0x75, 0x10,                     /*          Report Size (16),               */
		0x55, 0x0E,                     /*          Unit Exponent (14),             */
		0x65, 0x13,                     /*          Unit (Inch),                    */
		0x09, 0x30,                     /*          Usage (X),                      */
		0x35, 0x00,                     /*          Physical Minimum (0),           */
		0x46, 0xA3, 0x01,               /*          Physical Maximum (419),         */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x46, 0xF6, 0x00,               /*          Physical Maximum (246),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x09, 0x31,                     /*          Usage (Y),                      */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x0D,                     /*          Usage Page (Digitizer),         */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x64,                     /*          Logical Maximum (100),          */
		0x95, 0x03,                     /*          Report Count (3),               */
		0xC0,                           /*      End Collection,                     */
		0x09, 0x22,                     /*      Usage (Finger),                     */
		0xA1, 0x02,                     /*      Collection (Logical),               */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x01,                     /*          Logical Maximum (1),            */
		0x09, 0x47,                     /*          Usage (Touch Valid),            */
		0x09, 0x42,                     /*          Usage (Tip Switch),             */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x81, 0x03,                     /*          Input (Constant, Variable),     */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x75, 0x04,                     /*          Report Size (4),                */
		0x25, 0x0F,                     /*          Logical Maximum (15),           */
		0x09, 0x51,                     /*          Usage (Contact Identifier),     */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x01,                     /*          Usage Page (Desktop),           */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x26, 0xE2, 0x0C,               /*          Logical Maximum (3298),         */
		0x75, 0x10,                     /*          Report Size (16),               */
		0x55, 0x0E,                     /*          Unit Exponent (14),             */
		0x65, 0x13,                     /*          Unit (Inch),                    */
		0x09, 0x30,                     /*          Usage (X),                      */
		0x35, 0x00,                     /*          Physical Minimum (0),           */
		0x46, 0xA3, 0x01,               /*          Physical Maximum (419),         */
		0x95, 0x01,                     /*          Report Count (1),               */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x46, 0xF6, 0x00,               /*          Physical Maximum (246),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x26, 0xB5, 0x07,               /*          Logical Maximum (1973),         */
		0x09, 0x31,                     /*          Usage (Y),                      */
		0x81, 0x02,                     /*          Input (Variable),               */
		0x05, 0x0D,                     /*          Usage Page (Digitizer),         */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x64,                     /*          Logical Maximum (100),          */
		0x95, 0x03,                     /*          Report Count (3),               */
		0xC0,                           /*      End Collection,                     */
		0x55, 0x0C,                     /*      Unit Exponent (12),                 */
		0x66, 0x01, 0x10,               /*      Unit (Seconds),                     */
		0x47, 0xFF, 0xFF, 0x00, 0x00,   /*      Physical Maximum (65535),           */
		0x27, 0xFF, 0xFF, 0x00, 0x00,   /*      Logical Maximum (65535),            */
		0x75, 0x10,                     /*      Report Size (16),                   */
		0x95, 0x01,                     /*      Report Count (1),                   */
		0x09, 0x56,                     /*      Usage (Scan Time),                  */
		0x81, 0x02,                     /*      Input (Variable),                   */
		0x09, 0x54,                     /*      Usage (Contact Count),              */
		0x25, 0x7F,                     /*      Logical Maximum (127),              */
		0x95, 0x01,                     /*      Report Count (1),                   */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0x81, 0x02,                     /*      Input (Variable),                   */
		0x05, 0x09,                     /*      Usage Page (Button),                */
		0x09, 0x01,                     /*      Usage (01h),                        */
		0x25, 0x01,                     /*      Logical Maximum (1),                */
		0x75, 0x01,                     /*      Report Size (1),                    */
		0x95, 0x01,                     /*      Report Count (1),                   */
		0x81, 0x02,                     /*      Input (Variable),                   */
		0x95, 0x07,                     /*      Report Count (7),                   */
		0x81, 0x03,                     /*      Input (Constant, Variable),         */
		0x09, 0xC5,                     /*      Usage (C5h),                        */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0x95, 0x02,                     /*      Report Count (2),                   */
		0x81, 0x03,                     /*      Input (Constant, Variable),         */
		0x05, 0x0D,                     /*      Usage Page (Digitizer),             */
		0x85, 0x02,                     /*      Report ID (2),                      */
		0x09, 0x55,                     /*      Usage (Contact Count Maximum),      */
		0x09, 0x59,                     /*      Usage (59h),                        */
		0x75, 0x04,                     /*      Report Size (4),                    */
		0x95, 0x02,                     /*      Report Count (2),                   */
		0x25, 0x0F,                     /*      Logical Maximum (15),               */
		0xB1, 0x02,                     /*      Feature (Variable),                 */
		0x85, 0x07,                     /*      Report ID (7),                      */
		0x09, 0x60,                     /*      Usage (60h),                        */
		0x75, 0x01,                     /*      Report Size (1),                    */
		0x95, 0x01,                     /*      Report Count (1),                   */
		0x15, 0x00,                     /*      Logical Minimum (0),                */
		0x25, 0x01,                     /*      Logical Maximum (1),                */
		0xB1, 0x02,                     /*      Feature (Variable),                 */
		0x95, 0x0F,                     /*      Report Count (15),                  */
		0xB1, 0x03,                     /*      Feature (Constant, Variable),       */
		0x06, 0x00, 0xFF,               /*      Usage Page (FF00h),                 */
		0x06, 0x00, 0xFF,               /*      Usage Page (FF00h),                 */
		0x85, 0x06,                     /*      Report ID (6),                      */
		0x09, 0xC5,                     /*      Usage (C5h),                        */
		0x15, 0x00,                     /*      Logical Minimum (0),                */
		0x26, 0xFF, 0x00,               /*      Logical Maximum (255),              */
		0x75, 0x08,                     /*      Report Size (8),                    */
		0x96, 0x00, 0x01,               /*      Report Count (256),                 */
		0xB1, 0x02,                     /*      Feature (Variable),                 */
		0xC0,                           /*  End Collection,                         */
		0x05, 0x0D,                     /*  Usage Page (Digitizer),                 */
		0x09, 0x0E,                     /*  Usage (Configuration),                  */
		0xA1, 0x01,                     /*  Collection (Application),               */
		0x85, 0x03,                     /*      Report ID (3),                      */
		0x09, 0x22,                     /*      Usage (Finger),                     */
		0xA1, 0x00,                     /*      Collection (Physical),              */
		0x09, 0x52,                     /*          Usage (Device Mode),            */
		0x15, 0x00,                     /*          Logical Minimum (0),            */
		0x25, 0x0A,                     /*          Logical Maximum (10),           */
		0x75, 0x10,                     /*          Report Size (16),               */
		0x95, 0x01,                     /*          Report Count (1),               */
		0xB1, 0x02,                     /*          Feature (Variable),             */
		0xC0,                           /*      End Collection,                     */
		0x09, 0x22,                     /*      Usage (Finger),                     */
		0xA1, 0x00,                     /*      Collection (Physical),              */
		0x85, 0x05,                     /*          Report ID (5),                  */
		0x09, 0x57,                     /*          Usage (57h),                    */
		0x09, 0x58,                     /*          Usage (58h),                    */
		0x75, 0x01,                     /*          Report Size (1),                */
		0x95, 0x02,                     /*          Report Count (2),               */
		0x25, 0x01,                     /*          Logical Maximum (1),            */
		0xB1, 0x02,                     /*          Feature (Variable),             */
		0x95, 0x0E,                     /*          Report Count (14),              */
		0xB1, 0x03,                     /*          Feature (Constant, Variable),   */
		0xC0,                           /*      End Collection,                     */
		0xC0                            /*  End Collection                          */
};

static const uint8_t certification_status_report[] = {
		0x02,
		0xfc, 0x28, 0xfe, 0x84, 0x40, 0xcb, 0x9a, 0x87, 0x0d, 0xbe, 0x57, 0x3c, 0xb6, 0x70, 0x09, 0x88,
		0x07, 0x97, 0x2d, 0x2b, 0xe3, 0x38, 0x34, 0xb6, 0x6c, 0xed, 0xb0, 0xf7, 0xe5, 0x9c, 0xf6, 0xc2,
		0x2e, 0x84, 0x1b, 0xe8, 0xb4, 0x51, 0x78, 0x43, 0x1f, 0x28, 0x4b, 0x7c, 0x2d, 0x53, 0xaf, 0xfc,
		0x47, 0x70, 0x1b, 0x59, 0x6f, 0x74, 0x43, 0xc4, 0xf3, 0x47, 0x18, 0x53, 0x1a, 0xa2, 0xa1, 0x71,
		0xc7, 0x95, 0x0e, 0x31, 0x55, 0x21, 0xd3, 0xb5, 0x1e, 0xe9, 0x0c, 0xba, 0xec, 0xb8, 0x89, 0x19,
		0x3e, 0xb3, 0xaf, 0x75, 0x81, 0x9d, 0x53, 0xb9, 0x41, 0x57, 0xf4, 0x6d, 0x39, 0x25, 0x29, 0x7c,
		0x87, 0xd9, 0xb4, 0x98, 0x45, 0x7d, 0xa7, 0x26, 0x9c, 0x65, 0x3b, 0x85, 0x68, 0x89, 0xd7, 0x3b,
		0xbd, 0xff, 0x14, 0x67, 0xf2, 0x2b, 0xf0, 0x2a, 0x41, 0x54, 0xf0, 0xfd, 0x2c, 0x66, 0x7c, 0xf8,
		0xc0, 0x8f, 0x33, 0x13, 0x03, 0xf1, 0xd3, 0xc1, 0x0b, 0x89, 0xd9, 0x1b, 0x62, 0xcd, 0x51, 0xb7,
		0x80, 0xb8, 0xaf, 0x3a, 0x10, 0xc1, 0x8a, 0x5b, 0xe8, 0x8a, 0x56, 0xf0, 0x8c, 0xaa, 0xfa, 0x35,
		0xe9, 0x42, 0xc4, 0xd8, 0x55, 0xc3, 0x38, 0xcc, 0x2b, 0x53, 0x5c, 0x69, 0x52, 0xd5, 0xc8, 0x73,
		0x02, 0x38, 0x7c, 0x73, 0xb6, 0x41, 0xe7, 0xff, 0x05, 0xd8, 0x2b, 0x79, 0x9a, 0xe2, 0x34, 0x60,
		0x8f, 0xa3, 0x32, 0x1f, 0x09, 0x78, 0x62, 0xbc, 0x80, 0xe3, 0x0f, 0xbd, 0x65, 0x20, 0x08, 0x13,
		0xc1, 0xe2, 0xee, 0x53, 0x2d, 0x86, 0x7e, 0xa7, 0x5a, 0xc5, 0xd3, 0x7d, 0x98, 0xbe, 0x31, 0x48,
		0x1f, 0xfb, 0xda, 0xaf, 0xa2, 0xa8, 0x6a, 0x89, 0xd6, 0xbf, 0xf2, 0xd3, 0x32, 0x2a, 0x9a, 0xe4,
		0xcf, 0x17, 0xb7, 0xb8, 0xf4, 0xe1, 0x33, 0x08, 0x24, 0x8b, 0xc4, 0x43, 0xa5, 0xe5, 0x24, 0xc2
};

#endif /* CONSTANTS_H_ */
